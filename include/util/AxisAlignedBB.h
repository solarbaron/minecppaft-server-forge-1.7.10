/**
 * AxisAlignedBB.h — Axis-aligned bounding box for collision detection.
 *
 * Java reference: net.minecraft.util.AxisAlignedBB
 *
 * Core collision primitive used by:
 *   - Entity bounding boxes and movement collision
 *   - Block collision boxes
 *   - Entity queries (getEntitiesWithinAABB)
 *   - Explosion entity damage area
 *   - Ray tracing (calculateIntercept)
 *
 * All methods match Java exactly — same coordinate comparisons,
 * same face numbering, same math.
 *
 * Value type by design — cheap copies, no heap allocation.
 * JNI readiness: 6 doubles, predictable layout.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Vec3 — 3D double vector (used by ray-cast)
// Java reference: net.minecraft.util.Vec3
// ═══════════════════════════════════════════════════════════════════════════

struct Vec3 {
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    double squareDistanceTo(const Vec3& other) const {
        double dx = other.x - x;
        double dy = other.y - y;
        double dz = other.z - z;
        return dx * dx + dy * dy + dz * dz;
    }

    double lengthVector() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalize() const {
        double len = lengthVector();
        if (len < 1.0e-8) return {0, 0, 0};
        return {x / len, y / len, z / len};
    }

    Vec3 addVector(double dx, double dy, double dz) const {
        return {x + dx, y + dy, z + dz};
    }

    Vec3 subtract(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    double distanceTo(const Vec3& other) const {
        return std::sqrt(squareDistanceTo(other));
    }

    // Java: getIntermediateWithXValue — find point on line where x = target
    std::optional<Vec3> getIntermediateWithXValue(const Vec3& end, double targetX) const {
        double dx = end.x - x;
        if (dx * dx < 1.0e-14) return std::nullopt;
        double t = (targetX - x) / dx;
        if (t < 0.0 || t > 1.0) return std::nullopt;
        return Vec3{x + dx * t, y + (end.y - y) * t, z + (end.z - z) * t};
    }

    std::optional<Vec3> getIntermediateWithYValue(const Vec3& end, double targetY) const {
        double dy = end.y - y;
        if (dy * dy < 1.0e-14) return std::nullopt;
        double t = (targetY - y) / dy;
        if (t < 0.0 || t > 1.0) return std::nullopt;
        return Vec3{x + (end.x - x) * t, y + dy * t, z + (end.z - z) * t};
    }

    std::optional<Vec3> getIntermediateWithZValue(const Vec3& end, double targetZ) const {
        double dz = end.z - z;
        if (dz * dz < 1.0e-14) return std::nullopt;
        double t = (targetZ - z) / dz;
        if (t < 0.0 || t > 1.0) return std::nullopt;
        return Vec3{x + (end.x - x) * t, y + (end.y - y) * t, z + dz * t};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MovingObjectPosition — Ray-cast hit result.
// Java reference: net.minecraft.util.MovingObjectPosition
// ═══════════════════════════════════════════════════════════════════════════

struct RayHitResult {
    Vec3 hitVec;
    int32_t sideHit;     // 0=down, 1=up, 2=north, 3=south, 4=west, 5=east
    int32_t blockX, blockY, blockZ;  // Block position (for block hits)
};

// ═══════════════════════════════════════════════════════════════════════════
// AxisAlignedBB — Axis-aligned bounding box.
// Java reference: net.minecraft.util.AxisAlignedBB
// ═══════════════════════════════════════════════════════════════════════════

struct AxisAlignedBB {
    double minX, minY, minZ;
    double maxX, maxY, maxZ;

    // ─── Construction ───

    AxisAlignedBB()
        : minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0) {}

    AxisAlignedBB(double x1, double y1, double z1, double x2, double y2, double z2)
        : minX(x1), minY(y1), minZ(z1), maxX(x2), maxY(y2), maxZ(z2) {}

    // Java: getBoundingBox (factory)
    static AxisAlignedBB create(double x1, double y1, double z1,
                                  double x2, double y2, double z2) {
        return {x1, y1, z1, x2, y2, z2};
    }

    // Java: setBounds
    AxisAlignedBB& setBounds(double x1, double y1, double z1,
                               double x2, double y2, double z2) {
        minX = x1; minY = y1; minZ = z1;
        maxX = x2; maxY = y2; maxZ = z2;
        return *this;
    }

    // Java: setBB
    void copyFrom(const AxisAlignedBB& other) {
        minX = other.minX; minY = other.minY; minZ = other.minZ;
        maxX = other.maxX; maxY = other.maxY; maxZ = other.maxZ;
    }

    // ─── Geometric operations (return new AABB) ───

    // Java: addCoord — extend in a direction
    AxisAlignedBB addCoord(double dx, double dy, double dz) const {
        double x1 = minX, y1 = minY, z1 = minZ;
        double x2 = maxX, y2 = maxY, z2 = maxZ;
        if (dx < 0.0) x1 += dx;
        if (dx > 0.0) x2 += dx;
        if (dy < 0.0) y1 += dy;
        if (dy > 0.0) y2 += dy;
        if (dz < 0.0) z1 += dz;
        if (dz > 0.0) z2 += dz;
        return {x1, y1, z1, x2, y2, z2};
    }

    // Java: expand — grow symmetrically
    AxisAlignedBB expand(double dx, double dy, double dz) const {
        return {minX - dx, minY - dy, minZ - dz,
                maxX + dx, maxY + dy, maxZ + dz};
    }

    // Java: contract — shrink symmetrically
    AxisAlignedBB contract(double dx, double dy, double dz) const {
        return {minX + dx, minY + dy, minZ + dz,
                maxX - dx, maxY - dy, maxZ - dz};
    }

    // Java: union — merge two AABBs
    AxisAlignedBB unionWith(const AxisAlignedBB& other) const {
        return {std::min(minX, other.minX), std::min(minY, other.minY),
                std::min(minZ, other.minZ), std::max(maxX, other.maxX),
                std::max(maxY, other.maxY), std::max(maxZ, other.maxZ)};
    }

    // Java: getOffsetBoundingBox — translate (new copy)
    AxisAlignedBB getOffset(double dx, double dy, double dz) const {
        return {minX + dx, minY + dy, minZ + dz,
                maxX + dx, maxY + dy, maxZ + dz};
    }

    // Java: offset — translate in place
    AxisAlignedBB& offset(double dx, double dy, double dz) {
        minX += dx; minY += dy; minZ += dz;
        maxX += dx; maxY += dy; maxZ += dz;
        return *this;
    }

    // Java: copy
    AxisAlignedBB copy() const { return *this; }

    // ─── Collision offset calculations (Java exact) ───

    // Java: calculateXOffset
    double calculateXOffset(const AxisAlignedBB& other, double motion) const {
        if (other.maxY <= minY || other.minY >= maxY) return motion;
        if (other.maxZ <= minZ || other.minZ >= maxZ) return motion;
        if (motion > 0.0 && other.maxX <= minX) {
            double d = minX - other.maxX;
            if (d < motion) motion = d;
        }
        if (motion < 0.0 && other.minX >= maxX) {
            double d = maxX - other.minX;
            if (d > motion) motion = d;
        }
        return motion;
    }

    // Java: calculateYOffset
    double calculateYOffset(const AxisAlignedBB& other, double motion) const {
        if (other.maxX <= minX || other.minX >= maxX) return motion;
        if (other.maxZ <= minZ || other.minZ >= maxZ) return motion;
        if (motion > 0.0 && other.maxY <= minY) {
            double d = minY - other.maxY;
            if (d < motion) motion = d;
        }
        if (motion < 0.0 && other.minY >= maxY) {
            double d = maxY - other.minY;
            if (d > motion) motion = d;
        }
        return motion;
    }

    // Java: calculateZOffset
    double calculateZOffset(const AxisAlignedBB& other, double motion) const {
        if (other.maxX <= minX || other.minX >= maxX) return motion;
        if (other.maxY <= minY || other.minY >= maxY) return motion;
        if (motion > 0.0 && other.maxZ <= minZ) {
            double d = minZ - other.maxZ;
            if (d < motion) motion = d;
        }
        if (motion < 0.0 && other.minZ >= maxZ) {
            double d = maxZ - other.minZ;
            if (d > motion) motion = d;
        }
        return motion;
    }

    // ─── Intersection tests ───

    // Java: intersectsWith — strict < comparison on maxes
    bool intersectsWith(const AxisAlignedBB& other) const {
        if (other.maxX <= minX || other.minX >= maxX) return false;
        if (other.maxY <= minY || other.minY >= maxY) return false;
        if (other.maxZ <= minZ || other.minZ >= maxZ) return false;
        return true;
    }

    // Java: isVecInside — strict < comparison (exclusive bounds)
    bool isVecInside(const Vec3& v) const {
        if (v.x <= minX || v.x >= maxX) return false;
        if (v.y <= minY || v.y >= maxY) return false;
        if (v.z <= minZ || v.z >= maxZ) return false;
        return true;
    }

    // ─── Metrics ───

    // Java: getAverageEdgeLength
    double getAverageEdgeLength() const {
        return ((maxX - minX) + (maxY - minY) + (maxZ - minZ)) / 3.0;
    }

    // ─── Ray-cast ───

    // Java: calculateIntercept — ray vs AABB intersection
    // Returns std::nullopt if no intersection
    std::optional<RayHitResult> calculateIntercept(const Vec3& start, const Vec3& end) const {
        auto vecMinX = start.getIntermediateWithXValue(end, minX);
        auto vecMaxX = start.getIntermediateWithXValue(end, maxX);
        auto vecMinY = start.getIntermediateWithYValue(end, minY);
        auto vecMaxY = start.getIntermediateWithYValue(end, maxY);
        auto vecMinZ = start.getIntermediateWithZValue(end, minZ);
        auto vecMaxZ = start.getIntermediateWithZValue(end, maxZ);

        // Filter by face containment
        if (vecMinX && !isVecInYZ(*vecMinX)) vecMinX = std::nullopt;
        if (vecMaxX && !isVecInYZ(*vecMaxX)) vecMaxX = std::nullopt;
        if (vecMinY && !isVecInXZ(*vecMinY)) vecMinY = std::nullopt;
        if (vecMaxY && !isVecInXZ(*vecMaxY)) vecMaxY = std::nullopt;
        if (vecMinZ && !isVecInXY(*vecMinZ)) vecMinZ = std::nullopt;
        if (vecMaxZ && !isVecInXY(*vecMaxZ)) vecMaxZ = std::nullopt;

        // Find closest hit
        std::optional<Vec3> closest;
        auto tryCloser = [&](const std::optional<Vec3>& candidate) {
            if (candidate && (!closest || start.squareDistanceTo(*candidate) < start.squareDistanceTo(*closest))) {
                closest = candidate;
            }
        };
        tryCloser(vecMinX); tryCloser(vecMaxX);
        tryCloser(vecMinY); tryCloser(vecMaxY);
        tryCloser(vecMinZ); tryCloser(vecMaxZ);

        if (!closest) return std::nullopt;

        // Determine which face was hit
        // Java: 4=west(-X), 5=east(+X), 0=down(-Y), 1=up(+Y), 2=north(-Z), 3=south(+Z)
        int32_t side = -1;
        if (closest == vecMinX) side = 4;
        if (closest == vecMaxX) side = 5;
        if (closest == vecMinY) side = 0;
        if (closest == vecMaxY) side = 1;
        if (closest == vecMinZ) side = 2;
        if (closest == vecMaxZ) side = 3;

        RayHitResult result;
        result.hitVec = *closest;
        result.sideHit = side;
        result.blockX = 0;
        result.blockY = 0;
        result.blockZ = 0;
        return result;
    }

    // ─── String representation ───
    std::string toString() const {
        return "box[" + std::to_string(minX) + ", " + std::to_string(minY) + ", " +
               std::to_string(minZ) + " -> " + std::to_string(maxX) + ", " +
               std::to_string(maxY) + ", " + std::to_string(maxZ) + "]";
    }

private:
    // Face containment checks for ray-cast (using >= <= inclusive)
    bool isVecInYZ(const Vec3& v) const {
        return v.y >= minY && v.y <= maxY && v.z >= minZ && v.z <= maxZ;
    }
    bool isVecInXZ(const Vec3& v) const {
        return v.x >= minX && v.x <= maxX && v.z >= minZ && v.z <= maxZ;
    }
    bool isVecInXY(const Vec3& v) const {
        return v.x >= minX && v.x <= maxX && v.y >= minY && v.y <= maxY;
    }
};

} // namespace mccpp
