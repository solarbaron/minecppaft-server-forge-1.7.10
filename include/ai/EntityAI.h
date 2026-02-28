/**
 * EntityAI.h — Entity AI system: PathPoint, PathEntity, EntityAIBase,
 * EntityAITasks, and basic AI task implementations.
 *
 * Java references:
 *   - net.minecraft.pathfinding.PathPoint
 *   - net.minecraft.pathfinding.PathEntity
 *   - net.minecraft.entity.ai.EntityAIBase
 *   - net.minecraft.entity.ai.EntityAITasks
 *   - net.minecraft.entity.ai.EntityAIWander
 *   - net.minecraft.entity.ai.EntityAILookIdle
 *   - net.minecraft.entity.ai.EntityAISwimming
 *   - net.minecraft.entity.ai.EntityAIPanic
 *
 * Thread safety:
 *   - AI tasks execute on the server tick thread only.
 *   - PathPoint/PathEntity are value types, no shared state.
 *   - EntityAITasks is per-entity, single-threaded access.
 *
 * JNI readiness: Simple class hierarchy with virtual methods.
 * AI tasks can be enumerated by index for JVM binding.
 */
#pragma once

#include <cstdint>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PathPoint — A single node in the pathfinding graph.
// Java reference: net.minecraft.pathfinding.PathPoint
//
// Block-aligned coordinates with A* cost fields.
// Hash function matches Java's exactly for compatibility.
// ═══════════════════════════════════════════════════════════════════════════

struct PathPoint {
    int32_t x, y, z;

    // A* fields
    int32_t index = -1;            // Position in open set (-1 = not in set)
    float totalPathDistance = 0.0f; // g: cost from start to this node
    float distanceToNext = 0.0f;   // h: heuristic cost to end
    float distanceToTarget = 0.0f; // f = g + h
    int32_t previousIndex = -1;    // Index of previous node in path (backtrack)
    bool visited = false;

    PathPoint() : x(0), y(0), z(0) {}
    PathPoint(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}

    // Java: PathPoint.makeHash(int, int, int)
    // Hash encoding: y(8 bits) | x(15 bits, shifted 8) | z(15 bits, shifted 24) | sign bits
    static int32_t makeHash(int32_t x, int32_t y, int32_t z) {
        return (y & 0xFF)
             | ((x & 0x7FFF) << 8)
             | ((z & 0x7FFF) << 24)
             | (x < 0 ? static_cast<int32_t>(0x80000000) : 0)
             | (z < 0 ? 0x8000 : 0);
    }

    int32_t getHash() const { return makeHash(x, y, z); }

    // Java: PathPoint.distanceTo(PathPoint)
    float distanceTo(const PathPoint& other) const {
        float dx = static_cast<float>(other.x - x);
        float dy = static_cast<float>(other.y - y);
        float dz = static_cast<float>(other.z - z);
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // Java: PathPoint.distanceToSquared(PathPoint)
    float distanceToSquared(const PathPoint& other) const {
        float dx = static_cast<float>(other.x - x);
        float dy = static_cast<float>(other.y - y);
        float dz = static_cast<float>(other.z - z);
        return dx * dx + dy * dy + dz * dz;
    }

    bool operator==(const PathPoint& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// PathEntity — A sequence of PathPoints forming a complete path.
// Java reference: net.minecraft.pathfinding.PathEntity
//
// Tracks current position along the path with incrementPathIndex().
// ═══════════════════════════════════════════════════════════════════════════

class PathEntity {
public:
    PathEntity() = default;

    explicit PathEntity(std::vector<PathPoint> points)
        : points_(std::move(points)), pathLength_(static_cast<int32_t>(points_.size())) {}

    // Java: PathEntity.incrementPathIndex()
    void incrementPathIndex() { ++currentPathIndex_; }

    // Java: PathEntity.isFinished()
    bool isFinished() const { return currentPathIndex_ >= pathLength_; }

    // Java: PathEntity.getFinalPathPoint()
    const PathPoint* getFinalPathPoint() const {
        if (pathLength_ > 0) return &points_[pathLength_ - 1];
        return nullptr;
    }

    // Java: PathEntity.getPathPointFromIndex(int)
    const PathPoint& getPathPointFromIndex(int32_t index) const {
        return points_[index];
    }

    // Java: PathEntity.getCurrentPathLength()
    int32_t getCurrentPathLength() const { return pathLength_; }
    void setCurrentPathLength(int32_t len) { pathLength_ = len; }

    // Java: PathEntity.getCurrentPathIndex()
    int32_t getCurrentPathIndex() const { return currentPathIndex_; }
    void setCurrentPathIndex(int32_t idx) { currentPathIndex_ = idx; }

    // Java: PathEntity.getPosition(Entity) — simplified to return block coords
    // with entity width offset
    void getPosition(float entityWidth, double& outX, double& outY, double& outZ) const {
        if (currentPathIndex_ < pathLength_) {
            double offset = static_cast<double>(static_cast<int32_t>(entityWidth + 1.0f)) * 0.5;
            outX = static_cast<double>(points_[currentPathIndex_].x) + offset;
            outY = static_cast<double>(points_[currentPathIndex_].y);
            outZ = static_cast<double>(points_[currentPathIndex_].z) + offset;
        }
    }

    // Java: PathEntity.isSamePath(PathEntity)
    bool isSamePath(const PathEntity& other) const {
        if (static_cast<int32_t>(other.points_.size()) != static_cast<int32_t>(points_.size()))
            return false;
        for (size_t i = 0; i < points_.size(); ++i) {
            if (!(points_[i] == other.points_[i])) return false;
        }
        return true;
    }

    bool isEmpty() const { return points_.empty(); }

private:
    std::vector<PathPoint> points_;
    int32_t currentPathIndex_ = 0;
    int32_t pathLength_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityAIBase — Base class for all AI tasks.
// Java reference: net.minecraft.entity.ai.EntityAIBase
//
// Mutex bits control which tasks can run simultaneously.
// Bit-compatible tasks (mutexBits & otherBits == 0) can coexist.
// ═══════════════════════════════════════════════════════════════════════════

class EntityAIBase {
public:
    virtual ~EntityAIBase() = default;

    // Java: EntityAIBase.shouldExecute() — abstract
    virtual bool shouldExecute() = 0;

    // Java: EntityAIBase.continueExecuting() — defaults to shouldExecute()
    virtual bool continueExecuting() { return shouldExecute(); }

    // Java: EntityAIBase.isInterruptible() — can higher priority tasks interrupt
    virtual bool isInterruptible() const { return true; }

    // Java: EntityAIBase.startExecuting() — called when task begins
    virtual void startExecuting() {}

    // Java: EntityAIBase.resetTask() — called when task ends
    virtual void resetTask() {}

    // Java: EntityAIBase.updateTask() — called each tick while running
    virtual void updateTask() {}

    // Java: EntityAIBase.setMutexBits(int) / getMutexBits()
    void setMutexBits(int32_t bits) { mutexBits_ = bits; }
    int32_t getMutexBits() const { return mutexBits_; }

    // Name for debugging (replaces Java class.getSimpleName())
    virtual std::string getName() const { return "EntityAIBase"; }

private:
    int32_t mutexBits_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityAITasks — Priority-based AI task scheduler.
// Java reference: net.minecraft.entity.ai.EntityAITasks
//
// Tick logic (onUpdateTasks):
//   Every 3 ticks: full evaluation — stop incompatible/finished tasks, start new ones
//   Other ticks: just check if executing tasks should stop
//   Then: tick all executing tasks
//
// Priority: lower number = higher priority (can interrupt higher numbers)
// Mutex: bit-mask compatibility check between concurrent tasks
// ═══════════════════════════════════════════════════════════════════════════

class EntityAITasks {
public:
    struct TaskEntry {
        int32_t priority;
        std::shared_ptr<EntityAIBase> action;
        bool executing = false;
    };

    EntityAITasks() = default;

    // Java: EntityAITasks.addTask(int, EntityAIBase)
    void addTask(int32_t priority, std::shared_ptr<EntityAIBase> task);

    // Java: EntityAITasks.removeTask(EntityAIBase)
    void removeTask(EntityAIBase* task);

    // Java: EntityAITasks.onUpdateTasks()
    // Called once per tick by the entity
    void onUpdateTasks();

    // Access for debugging
    const std::vector<TaskEntry>& getTaskEntries() const { return taskEntries_; }

private:
    // Java: EntityAITasks.canContinue(EntityAITaskEntry)
    bool canContinue(const TaskEntry& entry) const;

    // Java: EntityAITasks.canUse(EntityAITaskEntry)
    bool canUse(const TaskEntry& entry) const;

    // Java: EntityAITasks.areTasksCompatible(entry1, entry2)
    static bool areTasksCompatible(const TaskEntry& a, const TaskEntry& b) {
        return (a.action->getMutexBits() & b.action->getMutexBits()) == 0;
    }

    std::vector<TaskEntry> taskEntries_;
    int32_t tickCount_ = 0;
    // Java: private int tickRate = 3
    static constexpr int32_t TICK_RATE = 3;
};

// ═══════════════════════════════════════════════════════════════════════════
// Basic AI Tasks — Common mob behaviors
// ═══════════════════════════════════════════════════════════════════════════

// ─── EntityAISwimming ──────────────────────────────────────────────────────
// Java reference: net.minecraft.entity.ai.EntityAISwimming
// Makes entity jump when in water/lava. Mutex bit 4.

class EntityAISwimming : public EntityAIBase {
public:
    EntityAISwimming() { setMutexBits(4); }
    bool shouldExecute() override { return inWater_; }
    void updateTask() override { /* would set entity.motionY += 0.04 */ }
    std::string getName() const override { return "EntityAISwimming"; }

    // Set by entity update
    void setInWater(bool inWater) { inWater_ = inWater; }
private:
    bool inWater_ = false;
};

// ─── EntityAIWander ────────────────────────────────────────────────────────
// Java reference: net.minecraft.entity.ai.EntityAIWander
// Random wandering. Mutex bit 1. Speed parameter.

class EntityAIWander : public EntityAIBase {
public:
    explicit EntityAIWander(double speed = 1.0)
        : speed_(speed) { setMutexBits(1); }

    bool shouldExecute() override;
    bool continueExecuting() override { return !pathFinished_; }
    void startExecuting() override { pathFinished_ = false; }
    void resetTask() override { pathFinished_ = true; }
    void updateTask() override;
    std::string getName() const override { return "EntityAIWander"; }

    double getSpeed() const { return speed_; }

private:
    double speed_;
    bool pathFinished_ = true;
    int32_t wanderTimer_ = 0;
};

// ─── EntityAILookIdle ──────────────────────────────────────────────────────
// Java reference: net.minecraft.entity.ai.EntityAILookIdle
// Random idle head rotation. Mutex bit 3.

class EntityAILookIdle : public EntityAIBase {
public:
    EntityAILookIdle() { setMutexBits(3); }

    bool shouldExecute() override;
    bool continueExecuting() override { return idleTime_ > 0; }
    void startExecuting() override;
    void updateTask() override;
    std::string getName() const override { return "EntityAILookIdle"; }

private:
    double lookX_ = 0.0, lookZ_ = 0.0;
    int32_t idleTime_ = 0;
};

// ─── EntityAIPanic ─────────────────────────────────────────────────────────
// Java reference: net.minecraft.entity.ai.EntityAIPanic
// Fast fleeing when hurt. Mutex bit 1. Speed multiplier.

class EntityAIPanic : public EntityAIBase {
public:
    explicit EntityAIPanic(double speed = 2.0)
        : speed_(speed) { setMutexBits(1); }

    bool shouldExecute() override { return isHurt_; }
    bool continueExecuting() override { return !pathFinished_; }
    void startExecuting() override { pathFinished_ = false; }
    void resetTask() override { pathFinished_ = true; isHurt_ = false; }
    void updateTask() override;
    std::string getName() const override { return "EntityAIPanic"; }

    void setHurt(bool hurt) { isHurt_ = hurt; }

private:
    double speed_;
    bool isHurt_ = false;
    bool pathFinished_ = true;
};

// ─── EntityAIWatchClosest ──────────────────────────────────────────────────
// Java reference: net.minecraft.entity.ai.EntityAIWatchClosest
// Look at the nearest entity of a given type. Mutex bit 2.

class EntityAIWatchClosest : public EntityAIBase {
public:
    explicit EntityAIWatchClosest(float maxDist = 8.0f, float chance = 0.02f)
        : maxDistance_(maxDist), chance_(chance) { setMutexBits(2); }

    bool shouldExecute() override;
    bool continueExecuting() override { return lookTime_ > 0; }
    void startExecuting() override;
    void resetTask() override { lookTime_ = 0; }
    void updateTask() override;
    std::string getName() const override { return "EntityAIWatchClosest"; }

    // Set by entity when it finds a target
    void setTarget(double targetX, double targetY, double targetZ) {
        hasTarget_ = true;
        targetX_ = targetX; targetY_ = targetY; targetZ_ = targetZ;
    }
    void clearTarget() { hasTarget_ = false; }

private:
    float maxDistance_;
    float chance_;
    bool hasTarget_ = false;
    double targetX_ = 0, targetY_ = 0, targetZ_ = 0;
    int32_t lookTime_ = 0;
};

} // namespace mccpp
