/**
 * PathFinder.h — A* pathfinding system for entity AI.
 *
 * Java references:
 *   - net.minecraft.pathfinding.PathPoint — Node in the A* graph
 *   - net.minecraft.pathfinding.Path — Binary min-heap (open set)
 *   - net.minecraft.pathfinding.PathEntity — Result waypoint list
 *   - net.minecraft.pathfinding.PathFinder — A* search algorithm
 *
 * The pathfinder uses grid-aligned (block) coordinates. Each PathPoint
 * is a 3D integer position with A* state (totalDistance, distanceToNext,
 * distanceToTarget, previous, visited). The open set is a binary min-heap
 * sorted by distanceToTarget (f-score).
 *
 * Neighbor expansion: 4-directional (N/S/E/W), with vertical movement
 * (stepping up 1 block, falling down with max fall height check).
 *
 * Block access is abstracted via a callback for thread-safe access to
 * the world's block data from any thread.
 *
 * JNI readiness: Simple structs, no opaque pointers.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PathPoint — Single node in the pathfinding graph.
// Java reference: net.minecraft.pathfinding.PathPoint
// ═══════════════════════════════════════════════════════════════════════════

struct PathPoint {
    int32_t x, y, z;
    int32_t hash;

    // A* state
    int32_t index = -1;              // -1 = not in heap
    float totalPathDistance = 0.0f;   // g-score
    float distanceToNext = 0.0f;     // Heuristic to target
    float distanceToTarget = 0.0f;   // f-score (g + h)
    int32_t previousIdx = -1;        // Index of previous node in result
    bool visited = false;

    PathPoint() : x(0), y(0), z(0), hash(0) {}

    PathPoint(int32_t x, int32_t y, int32_t z)
        : x(x), y(y), z(z), hash(makeHash(x, y, z)) {}

    // Java: PathPoint.makeHash — pack 3D coords into unique int
    // y&0xFF | (x&0x7FFF)<<8 | (z&0x7FFF)<<24 | signbits
    static int32_t makeHash(int32_t x, int32_t y, int32_t z) {
        return (y & 0xFF) |
               ((x & 0x7FFF) << 8) |
               ((z & 0x7FFF) << 24) |
               (x < 0 ? static_cast<int32_t>(0x80000000u) : 0) |
               (z < 0 ? 0x8000 : 0);
    }

    // Java: distanceTo — euclidean distance
    float distanceTo(const PathPoint& other) const {
        float dx = static_cast<float>(other.x - x);
        float dy = static_cast<float>(other.y - y);
        float dz = static_cast<float>(other.z - z);
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // Java: distanceToSquared
    float distanceToSquared(const PathPoint& other) const {
        float dx = static_cast<float>(other.x - x);
        float dy = static_cast<float>(other.y - y);
        float dz = static_cast<float>(other.z - z);
        return dx * dx + dy * dy + dz * dz;
    }

    bool isAssigned() const { return index >= 0; }

    bool operator==(const PathPoint& o) const {
        return hash == o.hash && x == o.x && y == o.y && z == o.z;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PathEntity — Result of pathfinding: ordered list of waypoints.
// Java reference: net.minecraft.pathfinding.PathEntity
// ═══════════════════════════════════════════════════════════════════════════

struct PathEntity {
    std::vector<PathPoint> points;
    int32_t currentIndex = 0;

    bool isFinished() const {
        return currentIndex >= static_cast<int32_t>(points.size());
    }

    void incrementIndex() { ++currentIndex; }

    const PathPoint* getCurrentPoint() const {
        if (currentIndex < static_cast<int32_t>(points.size())) {
            return &points[static_cast<size_t>(currentIndex)];
        }
        return nullptr;
    }

    const PathPoint* getFinalPoint() const {
        if (!points.empty()) return &points.back();
        return nullptr;
    }

    // Java: getVectorFromIndex — get centered position for entity width
    struct Vec3 { double x, y, z; };

    Vec3 getPosition(float entityWidth, int32_t idx) const {
        const auto& p = points[static_cast<size_t>(idx)];
        double offset = static_cast<double>(static_cast<int32_t>(entityWidth + 1.0f)) * 0.5;
        return { static_cast<double>(p.x) + offset,
                 static_cast<double>(p.y),
                 static_cast<double>(p.z) + offset };
    }

    Vec3 getCurrentPosition(float entityWidth) const {
        return getPosition(entityWidth, currentIndex);
    }

    bool isSamePath(const PathEntity& other) const {
        if (points.size() != other.points.size()) return false;
        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].x != other.points[i].x ||
                points[i].y != other.points[i].y ||
                points[i].z != other.points[i].z) return false;
        }
        return true;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PathHeap — Binary min-heap for the A* open set.
// Java reference: net.minecraft.pathfinding.Path
// ═══════════════════════════════════════════════════════════════════════════

class PathHeap {
public:
    void clear() {
        count_ = 0;
        heap_.clear();
    }

    bool isEmpty() const { return count_ == 0; }

    // Java: addPoint
    void addPoint(PathPoint* point) {
        if (point->index >= 0) return; // Already in heap

        if (count_ >= static_cast<int32_t>(heap_.size())) {
            heap_.resize(heap_.empty() ? 1024 : heap_.size() * 2);
        }
        heap_[static_cast<size_t>(count_)] = point;
        point->index = count_;
        sortBack(count_);
        ++count_;
    }

    // Java: dequeue — extract minimum
    PathPoint* dequeue() {
        PathPoint* result = heap_[0];
        --count_;
        heap_[0] = heap_[static_cast<size_t>(count_)];
        heap_[static_cast<size_t>(count_)] = nullptr;
        if (count_ > 0) {
            heap_[0]->index = 0;
            sortForward(0);
        }
        result->index = -1;
        return result;
    }

    // Java: changeDistance
    void changeDistance(PathPoint* point, float newDistance) {
        float old = point->distanceToTarget;
        point->distanceToTarget = newDistance;
        if (newDistance < old) {
            sortBack(point->index);
        } else {
            sortForward(point->index);
        }
    }

private:
    // Java: sortBack — bubble up
    void sortBack(int32_t idx) {
        PathPoint* point = heap_[static_cast<size_t>(idx)];
        float dist = point->distanceToTarget;
        while (idx > 0) {
            int32_t parent = (idx - 1) >> 1;
            PathPoint* parentPoint = heap_[static_cast<size_t>(parent)];
            if (dist >= parentPoint->distanceToTarget) break;
            heap_[static_cast<size_t>(idx)] = parentPoint;
            parentPoint->index = idx;
            idx = parent;
        }
        heap_[static_cast<size_t>(idx)] = point;
        point->index = idx;
    }

    // Java: sortForward — bubble down
    void sortForward(int32_t idx) {
        PathPoint* point = heap_[static_cast<size_t>(idx)];
        float dist = point->distanceToTarget;
        while (true) {
            int32_t left = 1 + (idx << 1);
            int32_t right = left + 1;
            if (left >= count_) break;

            PathPoint* leftPoint = heap_[static_cast<size_t>(left)];
            float leftDist = leftPoint->distanceToTarget;

            float rightDist;
            PathPoint* rightPoint;
            if (right >= count_) {
                rightPoint = nullptr;
                rightDist = std::numeric_limits<float>::infinity();
            } else {
                rightPoint = heap_[static_cast<size_t>(right)];
                rightDist = rightPoint->distanceToTarget;
            }

            if (leftDist < rightDist) {
                if (leftDist >= dist) break;
                heap_[static_cast<size_t>(idx)] = leftPoint;
                leftPoint->index = idx;
                idx = left;
            } else {
                if (rightDist >= dist) break;
                heap_[static_cast<size_t>(idx)] = rightPoint;
                rightPoint->index = idx;
                idx = right;
            }
        }
        heap_[static_cast<size_t>(idx)] = point;
        point->index = idx;
    }

    std::vector<PathPoint*> heap_;
    int32_t count_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// BlockStandability — Result of canEntityStandAt check.
// ═══════════════════════════════════════════════════════════════════════════

enum class BlockStandability : int32_t {
    Blocked = 0,
    Passable = 1,
    Open = 2,           // Walkable with special blocks (trapdoor, water)
    FenceGate = -3,
    Trapdoor = -4,
    Water = -1,
    Lava = -2,
};

// ═══════════════════════════════════════════════════════════════════════════
// PathFinder — A* pathfinding algorithm.
// Java reference: net.minecraft.pathfinding.PathFinder
//
// Block access is abstracted through a callback that returns
// BlockStandability for a given position and entity size.
// ═══════════════════════════════════════════════════════════════════════════

class PathFinder {
public:
    // Callback: (x, y, z, entitySizeX, entitySizeY, entitySizeZ) → standability
    using BlockCheckFn = std::function<int32_t(int32_t, int32_t, int32_t,
                                                 int32_t, int32_t, int32_t)>;

    struct EntityInfo {
        double posX, posY, posZ;      // Current position
        float width, height;           // Entity dimensions
        int32_t maxFallHeight = 3;     // Default max fall
        bool isInWater = false;
    };

    PathFinder(BlockCheckFn blockCheck, bool canPassOpenDoors,
               bool canPassClosedDoors, bool avoidsWater, bool canSwim)
        : blockCheck_(std::move(blockCheck)),
          canPassOpenDoors_(canPassOpenDoors),
          canPassClosedDoors_(canPassClosedDoors),
          avoidsWater_(avoidsWater),
          canSwim_(canSwim) {}

    // Java: createEntityPathTo(Entity, double, double, double, float)
    std::optional<PathEntity> findPath(const EntityInfo& entity,
                                        double targetX, double targetY, double targetZ,
                                        float maxDistance) {
        heap_.clear();
        pointMap_.clear();
        points_.clear();

        bool savedAvoidsWater = avoidsWater_;

        // Start Y position
        int32_t startY = static_cast<int32_t>(std::floor(entity.posY + 0.5));
        // Note: water swimming adjustment would go here

        int32_t startX = static_cast<int32_t>(std::floor(entity.posX));
        int32_t startZ = static_cast<int32_t>(std::floor(entity.posZ));

        // Entity size for collision
        int32_t sizeX = static_cast<int32_t>(std::floor(entity.width + 1.0f));
        int32_t sizeY = static_cast<int32_t>(std::floor(entity.height + 1.0f));
        int32_t sizeZ = sizeX;

        PathPoint& start = openPoint(startX, startY, startZ);
        int32_t endX = static_cast<int32_t>(std::floor(targetX - static_cast<double>(entity.width) / 2.0));
        int32_t endY = static_cast<int32_t>(std::floor(targetY));
        int32_t endZ = static_cast<int32_t>(std::floor(targetZ - static_cast<double>(entity.width) / 2.0));
        PathPoint& end = openPoint(endX, endY, endZ);
        PathPoint entitySize(sizeX, sizeY, sizeZ);

        auto result = aStarSearch(entity, start, end, entitySize, maxDistance);
        avoidsWater_ = savedAvoidsWater;
        return result;
    }

private:
    // Java: addToPath — core A* loop
    std::optional<PathEntity> aStarSearch(const EntityInfo& entity,
                                            PathPoint& start, PathPoint& target,
                                            const PathPoint& entitySize, float maxDist) {
        start.totalPathDistance = 0.0f;
        start.distanceToNext = start.distanceToSquared(target);
        start.distanceToTarget = start.distanceToNext;

        heap_.clear();
        heap_.addPoint(&start);

        PathPoint* closest = &start;

        while (!heap_.isEmpty()) {
            PathPoint* current = heap_.dequeue();

            if (*current == target) {
                return buildPath(start, *current);
            }

            if (current->distanceToSquared(target) < closest->distanceToSquared(target)) {
                closest = current;
            }

            current->visited = true;

            // Find neighbors
            PathPoint* neighbors[4];
            int32_t numNeighbors = findNeighbors(entity, *current, entitySize, target, maxDist, neighbors);

            for (int32_t i = 0; i < numNeighbors; ++i) {
                PathPoint* neighbor = neighbors[i];
                float tentativeG = current->totalPathDistance + current->distanceToSquared(*neighbor);

                if (neighbor->isAssigned() && tentativeG >= neighbor->totalPathDistance) continue;

                neighbor->previousIdx = getPointIndex(*current);
                neighbor->totalPathDistance = tentativeG;
                neighbor->distanceToNext = neighbor->distanceToSquared(target);

                if (neighbor->isAssigned()) {
                    heap_.changeDistance(neighbor, tentativeG + neighbor->distanceToNext);
                } else {
                    neighbor->distanceToTarget = tentativeG + neighbor->distanceToNext;
                    heap_.addPoint(neighbor);
                }
            }
        }

        // No path found to target — return path to closest point
        if (closest == &start) return std::nullopt;
        return buildPath(start, *closest);
    }

    // Java: findPathOptions — 4-directional neighbor expansion
    int32_t findNeighbors(const EntityInfo& entity, PathPoint& current,
                           const PathPoint& entitySize, const PathPoint& target,
                           float maxDist, PathPoint* neighbors[4]) {
        int32_t count = 0;

        // Check if we can step up
        int32_t stepUpHeight = 0;
        int32_t standResult = checkStandability(current.x, current.y + 1, current.z, entitySize);
        if (standResult == 1) stepUpHeight = 1;

        // 4 cardinal directions: +Z, -X, +X, -Z
        static const int32_t dx[] = { 0, -1, 1, 0 };
        static const int32_t dz[] = { 1, 0, 0, -1 };

        for (int32_t dir = 0; dir < 4; ++dir) {
            PathPoint* neighbor = getSafePoint(entity,
                current.x + dx[dir], current.y, current.z + dz[dir],
                entitySize, stepUpHeight);

            if (neighbor && !neighbor->visited &&
                neighbor->distanceTo(target) < maxDist) {
                neighbors[count++] = neighbor;
            }
        }
        return count;
    }

    // Java: getSafePoint — find safe landing point with step-up and fall-down
    PathPoint* getSafePoint(const EntityInfo& entity, int32_t x, int32_t y, int32_t z,
                              const PathPoint& entitySize, int32_t stepUpHeight) {
        PathPoint* result = nullptr;
        int32_t standability = checkStandability(x, y, z, entitySize);

        if (standability == 2) return &openPoint(x, y, z);
        if (standability == 1) result = &openPoint(x, y, z);

        // Try stepping up
        if (!result && stepUpHeight > 0 && standability != -3 && standability != -4) {
            if (checkStandability(x, y + stepUpHeight, z, entitySize) == 1) {
                result = &openPoint(x, y + stepUpHeight, z);
                y += stepUpHeight;
            }
        }

        // Try falling down
        if (result) {
            int32_t fallCount = 0;
            int32_t belowResult = 0;
            while (y > 0) {
                belowResult = checkStandability(x, y - 1, z, entitySize);
                if (avoidsWater_ && belowResult == -1) return nullptr;
                if (belowResult != 1) break;
                if (fallCount++ >= entity.maxFallHeight) return nullptr;
                --y;
                if (y > 0) result = &openPoint(x, y, z);
            }
            if (belowResult == -2) return nullptr; // Lava below
        }
        return result;
    }

    // Java: canEntityStandAt wrapper
    int32_t checkStandability(int32_t x, int32_t y, int32_t z, const PathPoint& entitySize) {
        if (blockCheck_) {
            return blockCheck_(x, y, z, entitySize.x, entitySize.y, entitySize.z);
        }
        return 1; // Default: passable
    }

    // Get/create a PathPoint
    PathPoint& openPoint(int32_t x, int32_t y, int32_t z) {
        int32_t hash = PathPoint::makeHash(x, y, z);
        auto it = pointMap_.find(hash);
        if (it != pointMap_.end()) {
            return points_[static_cast<size_t>(it->second)];
        }
        int32_t idx = static_cast<int32_t>(points_.size());
        points_.emplace_back(x, y, z);
        pointMap_[hash] = idx;
        return points_.back();
    }

    int32_t getPointIndex(const PathPoint& point) const {
        auto it = pointMap_.find(point.hash);
        return it != pointMap_.end() ? it->second : -1;
    }

    // Java: createEntityPath — trace back from end to start
    PathEntity buildPath(const PathPoint& start, const PathPoint& end) {
        PathEntity result;

        // Count nodes
        std::vector<int32_t> indices;
        int32_t endIdx = getPointIndex(end);
        int32_t current = endIdx;
        while (current >= 0) {
            indices.push_back(current);
            current = points_[static_cast<size_t>(current)].previousIdx;
        }

        // Reverse to get start→end order
        std::reverse(indices.begin(), indices.end());
        result.points.reserve(indices.size());
        for (int32_t idx : indices) {
            result.points.push_back(points_[static_cast<size_t>(idx)]);
        }
        return result;
    }

    // ─── State ───
    BlockCheckFn blockCheck_;
    bool canPassOpenDoors_;
    bool canPassClosedDoors_;
    bool avoidsWater_;
    bool canSwim_;

    PathHeap heap_;
    std::unordered_map<int32_t, int32_t> pointMap_; // hash → index in points_
    std::vector<PathPoint> points_;
};

} // namespace mccpp
