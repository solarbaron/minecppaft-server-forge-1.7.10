/**
 * EntityAI.cpp — Entity AI system implementation.
 *
 * Java references:
 *   net.minecraft.entity.ai.EntityAITasks — priority-based task scheduler
 *   net.minecraft.entity.ai.EntityAIWander — random wandering
 *   net.minecraft.entity.ai.EntityAILookIdle — idle head rotation
 *   net.minecraft.entity.ai.EntityAIWatchClosest — look at nearby entities
 *
 * Key behaviors preserved from Java:
 *   - EntityAITasks.onUpdateTasks: evaluates every TICK_RATE(3) ticks
 *   - canUse: mutex bit compatibility + priority-based interruption
 *   - Lower priority number = higher priority (can interrupt higher numbers)
 */

#include "ai/EntityAI.h"
#include <random>
#include <cmath>

namespace mccpp {

// ═════════════════════════════════════════════════════════════════════════════
// EntityAITasks
// ═════════════════════════════════════════════════════════════════════════════

void EntityAITasks::addTask(int32_t priority, std::shared_ptr<EntityAIBase> task) {
    // Java: EntityAITasks.addTask(int, EntityAIBase)
    taskEntries_.push_back({priority, std::move(task), false});
}

void EntityAITasks::removeTask(EntityAIBase* task) {
    // Java: EntityAITasks.removeTask(EntityAIBase)
    for (auto it = taskEntries_.begin(); it != taskEntries_.end(); ) {
        if (it->action.get() == task) {
            if (it->executing) {
                it->action->resetTask();
            }
            it = taskEntries_.erase(it);
        } else {
            ++it;
        }
    }
}

void EntityAITasks::onUpdateTasks() {
    // Java: EntityAITasks.onUpdateTasks()
    std::vector<TaskEntry*> newlyStarted;

    if (tickCount_++ % TICK_RATE == 0) {
        // Full evaluation tick — check all tasks
        for (auto& entry : taskEntries_) {
            if (entry.executing) {
                // Check if executing task should continue
                if (canUse(entry) && canContinue(entry)) {
                    continue;
                }
                // Stop this task
                entry.action->resetTask();
                entry.executing = false;
            }

            // Try to start non-executing tasks
            if (!entry.executing && canUse(entry) && entry.action->shouldExecute()) {
                newlyStarted.push_back(&entry);
                entry.executing = true;
            }
        }
    } else {
        // Non-evaluation tick — just check if executing tasks should stop
        for (auto& entry : taskEntries_) {
            if (entry.executing && !entry.action->continueExecuting()) {
                entry.action->resetTask();
                entry.executing = false;
            }
        }
    }

    // Start newly activated tasks
    for (auto* entry : newlyStarted) {
        entry->action->startExecuting();
    }

    // Tick all executing tasks
    for (auto& entry : taskEntries_) {
        if (entry.executing) {
            entry.action->updateTask();
        }
    }
}

bool EntityAITasks::canContinue(const TaskEntry& entry) const {
    // Java: EntityAITasks.canContinue(EntityAITaskEntry)
    return entry.action->continueExecuting();
}

bool EntityAITasks::canUse(const TaskEntry& entry) const {
    // Java: EntityAITasks.canUse(EntityAITaskEntry)
    // Check if this task's mutex bits are compatible with all currently
    // executing tasks, respecting priority-based interruption.
    for (const auto& other : taskEntries_) {
        if (&other == &entry) continue;

        if (entry.priority >= other.priority) {
            // Same or lower priority — must be compatible (mutex-wise) with executing tasks
            if (other.executing && !areTasksCompatible(entry, other)) {
                return false;
            }
        } else {
            // Higher priority (lower number) — can interrupt if interruptible
            if (other.executing && !other.action->isInterruptible()) {
                return false;
            }
        }
    }
    return true;
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityAIWander
// Java reference: net.minecraft.entity.ai.EntityAIWander
// ═════════════════════════════════════════════════════════════════════════════

// Thread-local RNG for AI decisions
static thread_local std::mt19937 aiRng(std::random_device{}());

bool EntityAIWander::shouldExecute() {
    // Java: EntityAIWander.shouldExecute()
    // In Java, this checks if the entity has no path and a random chance
    // Simplified: random chance per tick (1/120 chance ~ once every 6 seconds)
    std::uniform_int_distribution<int> dist(0, 119);
    return dist(aiRng) == 0;
}

void EntityAIWander::updateTask() {
    // Java: EntityAIWander.updateTask()
    // Decrement wander timer; when done, mark path finished
    ++wanderTimer_;
    if (wanderTimer_ > 60) { // ~3 seconds of wandering
        pathFinished_ = true;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityAILookIdle
// Java reference: net.minecraft.entity.ai.EntityAILookIdle
// ═════════════════════════════════════════════════════════════════════════════

bool EntityAILookIdle::shouldExecute() {
    // Java: EntityAILookIdle.shouldExecute()
    // Random chance: 1/10 per tick
    std::uniform_int_distribution<int> dist(0, 9);
    return dist(aiRng) == 0;
}

void EntityAILookIdle::startExecuting() {
    // Java: EntityAILookIdle.startExecuting()
    // Pick random look direction and duration
    std::uniform_real_distribution<double> angleDist(-M_PI, M_PI);
    std::uniform_int_distribution<int> timeDist(20, 60);

    double angle = angleDist(aiRng);
    lookX_ = std::cos(angle);
    lookZ_ = std::sin(angle);
    idleTime_ = timeDist(aiRng);
}

void EntityAILookIdle::updateTask() {
    // Java: EntityAILookIdle.updateTask()
    --idleTime_;
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityAIPanic
// Java reference: net.minecraft.entity.ai.EntityAIPanic
// ═════════════════════════════════════════════════════════════════════════════

void EntityAIPanic::updateTask() {
    // Java: EntityAIPanic.updateTask()
    // Panic continues until path is exhausted
    // In full implementation, would move entity along panic path at speed_
    // Simplified: panic for 60 ticks then stop
    static thread_local int32_t panicTimer = 0;
    ++panicTimer;
    if (panicTimer > 60) {
        pathFinished_ = true;
        panicTimer = 0;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// EntityAIWatchClosest
// Java reference: net.minecraft.entity.ai.EntityAIWatchClosest
// ═════════════════════════════════════════════════════════════════════════════

bool EntityAIWatchClosest::shouldExecute() {
    // Java: EntityAIWatchClosest.shouldExecute()
    // Check if there's a target and random chance
    if (!hasTarget_) return false;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(aiRng) < chance_;
}

void EntityAIWatchClosest::startExecuting() {
    // Java: EntityAIWatchClosest.startExecuting()
    std::uniform_int_distribution<int> dist(40, 80);
    lookTime_ = dist(aiRng);
}

void EntityAIWatchClosest::updateTask() {
    // Java: EntityAIWatchClosest.updateTask()
    --lookTime_;
}

} // namespace mccpp
