/**
 * EntityAITasks.h — Priority-based AI task scheduler for mob behavior.
 *
 * Java references:
 *   - net.minecraft.entity.ai.EntityAIBase — Abstract AI task interface
 *   - net.minecraft.entity.ai.EntityAITasks — Task scheduler with priority + mutex
 *   - net.minecraft.entity.ai.EntityAITasks$EntityAITaskEntry — Priority-wrapped task
 *
 * The AI system uses priority numbers (lower = higher priority) and
 * mutex bits for task compatibility. Tasks that share mutex bits cannot
 * run concurrently. Higher-priority non-interruptible tasks block
 * lower-priority tasks from starting.
 *
 * Tick throttling: shouldExecute() is only checked every tickRate ticks
 * (default: 3). continueExecuting() is checked every tick for active tasks.
 *
 * Not thread-safe by design: each mob's AI runs on a single tick thread.
 * JNI readiness: Clean virtual interface, predictable layout.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// EntityAIBase — Abstract base class for all AI tasks.
// Java reference: net.minecraft.entity.ai.EntityAIBase
// ═══════════════════════════════════════════════════════════════════════════

class EntityAIBase {
public:
    virtual ~EntityAIBase() = default;

    // Java: shouldExecute — called to check if the task should start
    virtual bool shouldExecute() = 0;

    // Java: continueExecuting — called each tick to check if the task should continue
    // Default: delegates to shouldExecute()
    virtual bool continueExecuting() { return shouldExecute(); }

    // Java: isInterruptible — can this task be interrupted by higher-priority tasks?
    // Default: true
    virtual bool isInterruptible() { return true; }

    // Java: startExecuting — called when the task starts
    virtual void startExecuting() {}

    // Java: resetTask — called when the task stops or is interrupted
    virtual void resetTask() {}

    // Java: updateTask — called every tick while the task is executing
    virtual void updateTask() {}

    // Java: mutexBits — bitmask for mutual exclusion
    void setMutexBits(int32_t bits) { mutexBits_ = bits; }
    int32_t getMutexBits() const { return mutexBits_; }

    // Debug name for profiler
    virtual std::string getName() const { return "EntityAIBase"; }

private:
    int32_t mutexBits_ = 0;
};

// ═══════════════════════════════════════════════════════════════════════════
// Common mutex bit constants used by vanilla AI tasks.
// ═══════════════════════════════════════════════════════════════════════════

namespace AIMutex {
    constexpr int32_t MOVE  = 1;   // Movement control
    constexpr int32_t LOOK  = 2;   // Head/look control
    constexpr int32_t JUMP  = 4;   // Jump control
    constexpr int32_t MOVE_LOOK = MOVE | LOOK;
    constexpr int32_t ALL   = MOVE | LOOK | JUMP;
}

// ═══════════════════════════════════════════════════════════════════════════
// EntityAITaskEntry — Task with priority metadata.
// Java reference: net.minecraft.entity.ai.EntityAITasks$EntityAITaskEntry
// ═══════════════════════════════════════════════════════════════════════════

struct EntityAITaskEntry {
    int32_t priority;
    std::shared_ptr<EntityAIBase> action;

    EntityAITaskEntry(int32_t prio, std::shared_ptr<EntityAIBase> task)
        : priority(prio), action(std::move(task)) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// EntityAITasks — Priority-based task scheduler.
// Java reference: net.minecraft.entity.ai.EntityAITasks
// ═══════════════════════════════════════════════════════════════════════════

class EntityAITasks {
public:
    EntityAITasks() = default;

    // Java: setTickRate — control shouldExecute() check frequency
    void setTickRate(int32_t rate) { tickRate_ = rate; }
    int32_t getTickRate() const { return tickRate_; }

    // Java: addTask
    void addTask(int32_t priority, std::shared_ptr<EntityAIBase> task) {
        taskEntries_.emplace_back(priority, std::move(task));
    }

    // Java: removeTask
    void removeTask(EntityAIBase* task) {
        for (auto it = taskEntries_.begin(); it != taskEntries_.end(); ) {
            if (it->action.get() == task) {
                // If it's currently executing, reset it
                auto execIt = std::find_if(executingEntries_.begin(), executingEntries_.end(),
                    [&](const EntityAITaskEntry* e) { return e->action.get() == task; });
                if (execIt != executingEntries_.end()) {
                    task->resetTask();
                    executingEntries_.erase(execIt);
                }
                it = taskEntries_.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Java: onUpdateTasks — main per-tick update
    void onUpdateTasks() {
        std::vector<EntityAITaskEntry*> newlyStarted;

        if (tickCount_++ % tickRate_ == 0) {
            // Full evaluation: check all tasks
            for (auto& entry : taskEntries_) {
                bool isExecuting = isEntryExecuting(&entry);

                if (isExecuting) {
                    // Already running — check if it should stop
                    if (canUse(&entry) && canContinue(&entry)) {
                        continue; // Keep running
                    }
                    // Stop it
                    entry.action->resetTask();
                    removeFromExecuting(&entry);
                }

                // Check if it can start
                if (canUse(&entry) && entry.action->shouldExecute()) {
                    newlyStarted.push_back(&entry);
                    executingEntries_.push_back(&entry);
                }
            }
        } else {
            // Between full evaluations: only check active tasks for stop conditions
            for (auto it = executingEntries_.begin(); it != executingEntries_.end(); ) {
                if (!(*it)->action->continueExecuting()) {
                    (*it)->action->resetTask();
                    it = executingEntries_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Start newly added tasks
        for (auto* entry : newlyStarted) {
            entry->action->startExecuting();
        }

        // Tick all executing tasks
        for (auto* entry : executingEntries_) {
            entry->action->updateTask();
        }
    }

    // Get number of executing tasks (for debugging/profiling)
    size_t getExecutingCount() const { return executingEntries_.size(); }
    size_t getTotalCount() const { return taskEntries_.size(); }

    // Check if a specific task type is running
    bool isTaskRunning(const EntityAIBase* task) const {
        for (const auto* entry : executingEntries_) {
            if (entry->action.get() == task) return true;
        }
        return false;
    }

private:
    // Java: canContinue
    bool canContinue(EntityAITaskEntry* entry) {
        return entry->action->continueExecuting();
    }

    // Java: canUse — check if this task can run given currently executing tasks
    bool canUse(EntityAITaskEntry* candidate) {
        for (auto& other : taskEntries_) {
            if (&other == candidate) continue;

            if (candidate->priority >= other.priority) {
                // Same or lower priority: check mutex compatibility
                if (isEntryExecuting(&other) && !areTasksCompatible(candidate, &other)) {
                    return false;
                }
            } else {
                // Higher priority than other: can preempt if other is interruptible
                if (isEntryExecuting(&other) && !other.action->isInterruptible()) {
                    return false;
                }
            }
        }
        return true;
    }

    // Java: areTasksCompatible — mutex bits check
    bool areTasksCompatible(const EntityAITaskEntry* a, const EntityAITaskEntry* b) const {
        return (a->action->getMutexBits() & b->action->getMutexBits()) == 0;
    }

    bool isEntryExecuting(const EntityAITaskEntry* entry) const {
        return std::find(executingEntries_.begin(), executingEntries_.end(), entry)
               != executingEntries_.end();
    }

    void removeFromExecuting(const EntityAITaskEntry* entry) {
        executingEntries_.erase(
            std::remove(executingEntries_.begin(), executingEntries_.end(), entry),
            executingEntries_.end());
    }

    // ─── State ───
    std::vector<EntityAITaskEntry> taskEntries_;
    std::vector<EntityAITaskEntry*> executingEntries_;  // Pointers into taskEntries_
    int32_t tickCount_ = 0;
    int32_t tickRate_ = 3;  // Java default
};

// ═══════════════════════════════════════════════════════════════════════════
// Common concrete AI task implementations (simplified server-side).
// Java references: net.minecraft.entity.ai.EntityAI*
// ═══════════════════════════════════════════════════════════════════════════

// EntityAISwimming — Swim when in water (mutex: JUMP)
class EntityAISwimming : public EntityAIBase {
public:
    EntityAISwimming() { setMutexBits(AIMutex::JUMP); }
    bool shouldExecute() override {
        return inWater_;    // Set externally by entity tick
    }
    void updateTask() override {
        // Entity jumps randomly when in water
        shouldJump_ = true;
    }
    std::string getName() const override { return "EntityAISwimming"; }

    bool inWater_ = false;
    bool shouldJump_ = false;
};

// EntityAIPanic — Run away when hurt (mutex: MOVE)
class EntityAIPanic : public EntityAIBase {
public:
    explicit EntityAIPanic(double speed) : speed_(speed) { setMutexBits(AIMutex::MOVE); }
    bool shouldExecute() override { return isHurt_; }
    bool continueExecuting() override { return hasTarget_; }
    void startExecuting() override { hasTarget_ = true; }
    void resetTask() override { hasTarget_ = false; }
    std::string getName() const override { return "EntityAIPanic"; }

    double speed_;
    bool isHurt_ = false;
    bool hasTarget_ = false;
};

// EntityAIWander — Random wandering (mutex: MOVE)
class EntityAIWander : public EntityAIBase {
public:
    explicit EntityAIWander(double speed) : speed_(speed) { setMutexBits(AIMutex::MOVE); }
    bool shouldExecute() override { return shouldWander_; }
    bool continueExecuting() override { return !reachedTarget_; }
    void startExecuting() override { reachedTarget_ = false; }
    void resetTask() override { reachedTarget_ = true; }
    std::string getName() const override { return "EntityAIWander"; }

    double speed_;
    bool shouldWander_ = false;
    bool reachedTarget_ = false;
};

// EntityAIWatchClosest — Look at nearby entity (mutex: LOOK)
class EntityAIWatchClosest : public EntityAIBase {
public:
    explicit EntityAIWatchClosest(float maxDist) : maxDistance_(maxDist) {
        setMutexBits(AIMutex::LOOK);
    }
    bool shouldExecute() override { return hasTarget_; }
    bool continueExecuting() override { return ticksRemaining_ > 0; }
    void startExecuting() override { ticksRemaining_ = 40 + (rand() % 40); }
    void updateTask() override { --ticksRemaining_; }
    void resetTask() override { hasTarget_ = false; ticksRemaining_ = 0; }
    std::string getName() const override { return "EntityAIWatchClosest"; }

    float maxDistance_;
    bool hasTarget_ = false;
    int32_t ticksRemaining_ = 0;
};

// EntityAILookIdle — Random head idle movement (mutex: LOOK)
class EntityAILookIdle : public EntityAIBase {
public:
    EntityAILookIdle() { setMutexBits(AIMutex::LOOK); }
    bool shouldExecute() override {
        return (rand() % 80) == 0; // 1.25% chance per check
    }
    bool continueExecuting() override { return ticksRemaining_ > 0; }
    void startExecuting() override {
        ticksRemaining_ = 20 + (rand() % 20);
        lookYaw_ = static_cast<float>(rand() % 360);
    }
    void updateTask() override { --ticksRemaining_; }
    void resetTask() override { ticksRemaining_ = 0; }
    std::string getName() const override { return "EntityAILookIdle"; }

    int32_t ticksRemaining_ = 0;
    float lookYaw_ = 0.0f;
};

// EntityAIAttackOnCollide — Melee attack (mutex: MOVE | LOOK)
class EntityAIAttackOnCollide : public EntityAIBase {
public:
    explicit EntityAIAttackOnCollide(double speed) : speed_(speed) {
        setMutexBits(AIMutex::MOVE_LOOK);
    }
    bool shouldExecute() override { return hasTarget_; }
    bool continueExecuting() override { return hasTarget_ && !targetDead_; }
    void startExecuting() override { attackCooldown_ = 0; }
    void updateTask() override {
        if (attackCooldown_ > 0) --attackCooldown_;
    }
    void resetTask() override { hasTarget_ = false; attackCooldown_ = 0; }
    bool isInterruptible() override { return true; }
    std::string getName() const override { return "EntityAIAttackOnCollide"; }

    double speed_;
    bool hasTarget_ = false;
    bool targetDead_ = false;
    int32_t attackCooldown_ = 0;
};

// EntityAIFollowParent — Baby follows parent (mutex: MOVE | LOOK)
class EntityAIFollowParent : public EntityAIBase {
public:
    explicit EntityAIFollowParent(double speed) : speed_(speed) {
        setMutexBits(AIMutex::MOVE_LOOK);
    }
    bool shouldExecute() override { return hasParent_ && isChild_; }
    bool continueExecuting() override { return hasParent_ && distToParent_ > 9.0; }
    void resetTask() override { hasParent_ = false; }
    std::string getName() const override { return "EntityAIFollowParent"; }

    double speed_;
    bool hasParent_ = false;
    bool isChild_ = false;
    double distToParent_ = 0.0;
};

// EntityAITempt — Follow player holding food (mutex: MOVE | LOOK)
class EntityAITempt : public EntityAIBase {
public:
    explicit EntityAITempt(double speed) : speed_(speed) {
        setMutexBits(AIMutex::MOVE_LOOK);
    }
    bool shouldExecute() override { return hasTemptingPlayer_; }
    bool continueExecuting() override {
        return hasTemptingPlayer_ && distToPlayer_ < 36.0; // 6 blocks squared
    }
    void resetTask() override { hasTemptingPlayer_ = false; }
    bool isInterruptible() override { return false; } // Cannot be interrupted
    std::string getName() const override { return "EntityAITempt"; }

    double speed_;
    bool hasTemptingPlayer_ = false;
    double distToPlayer_ = 0.0;
};

} // namespace mccpp
