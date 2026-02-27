/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util;

import io.netty.util.ResourceLeak;
import io.netty.util.ResourceLeakDetector;
import io.netty.util.Timeout;
import io.netty.util.Timer;
import io.netty.util.TimerTask;
import io.netty.util.internal.PlatformDependent;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class HashedWheelTimer
implements Timer {
    static final InternalLogger logger = InternalLoggerFactory.getInstance(HashedWheelTimer.class);
    private static final ResourceLeakDetector<HashedWheelTimer> leakDetector = new ResourceLeakDetector(HashedWheelTimer.class, 1, (long)(Runtime.getRuntime().availableProcessors() * 4));
    private final ResourceLeak leak;
    private final Worker worker = new Worker();
    final Thread workerThread;
    public static final int WORKER_STATE_INIT = 0;
    public static final int WORKER_STATE_STARTED = 1;
    public static final int WORKER_STATE_SHUTDOWN = 2;
    final AtomicInteger workerState = new AtomicInteger();
    final long tickDuration;
    final Set<HashedWheelTimeout>[] wheel;
    final int mask;
    final ReadWriteLock lock = new ReentrantReadWriteLock();
    volatile int wheelCursor;

    public HashedWheelTimer() {
        this(Executors.defaultThreadFactory());
    }

    public HashedWheelTimer(long tickDuration, TimeUnit unit) {
        this(Executors.defaultThreadFactory(), tickDuration, unit);
    }

    public HashedWheelTimer(long tickDuration, TimeUnit unit, int ticksPerWheel) {
        this(Executors.defaultThreadFactory(), tickDuration, unit, ticksPerWheel);
    }

    public HashedWheelTimer(ThreadFactory threadFactory) {
        this(threadFactory, 100L, TimeUnit.MILLISECONDS);
    }

    public HashedWheelTimer(ThreadFactory threadFactory, long tickDuration, TimeUnit unit) {
        this(threadFactory, tickDuration, unit, 512);
    }

    public HashedWheelTimer(ThreadFactory threadFactory, long tickDuration, TimeUnit unit, int ticksPerWheel) {
        if (threadFactory == null) {
            throw new NullPointerException("threadFactory");
        }
        if (unit == null) {
            throw new NullPointerException("unit");
        }
        if (tickDuration <= 0L) {
            throw new IllegalArgumentException("tickDuration must be greater than 0: " + tickDuration);
        }
        if (ticksPerWheel <= 0) {
            throw new IllegalArgumentException("ticksPerWheel must be greater than 0: " + ticksPerWheel);
        }
        this.wheel = HashedWheelTimer.createWheel(ticksPerWheel);
        this.mask = this.wheel.length - 1;
        this.tickDuration = unit.toNanos(tickDuration);
        if (this.tickDuration >= Long.MAX_VALUE / (long)this.wheel.length) {
            throw new IllegalArgumentException(String.format("tickDuration: %d (expected: 0 < tickDuration in nanos < %d", tickDuration, Long.MAX_VALUE / (long)this.wheel.length));
        }
        this.workerThread = threadFactory.newThread(this.worker);
        this.leak = leakDetector.open(this);
    }

    private static Set<HashedWheelTimeout>[] createWheel(int ticksPerWheel) {
        if (ticksPerWheel <= 0) {
            throw new IllegalArgumentException("ticksPerWheel must be greater than 0: " + ticksPerWheel);
        }
        if (ticksPerWheel > 0x40000000) {
            throw new IllegalArgumentException("ticksPerWheel may not be greater than 2^30: " + ticksPerWheel);
        }
        ticksPerWheel = HashedWheelTimer.normalizeTicksPerWheel(ticksPerWheel);
        Set[] wheel = new Set[ticksPerWheel];
        for (int i2 = 0; i2 < wheel.length; ++i2) {
            wheel[i2] = Collections.newSetFromMap(PlatformDependent.newConcurrentHashMap());
        }
        return wheel;
    }

    private static int normalizeTicksPerWheel(int ticksPerWheel) {
        int normalizedTicksPerWheel;
        for (normalizedTicksPerWheel = 1; normalizedTicksPerWheel < ticksPerWheel; normalizedTicksPerWheel <<= 1) {
        }
        return normalizedTicksPerWheel;
    }

    public void start() {
        switch (this.workerState.get()) {
            case 0: {
                if (!this.workerState.compareAndSet(0, 1)) break;
                this.workerThread.start();
                break;
            }
            case 1: {
                break;
            }
            case 2: {
                throw new IllegalStateException("cannot be started once stopped");
            }
            default: {
                throw new Error("Invalid WorkerState");
            }
        }
    }

    @Override
    public Set<Timeout> stop() {
        if (Thread.currentThread() == this.workerThread) {
            throw new IllegalStateException(HashedWheelTimer.class.getSimpleName() + ".stop() cannot be called from " + TimerTask.class.getSimpleName());
        }
        if (!this.workerState.compareAndSet(1, 2)) {
            this.workerState.set(2);
            if (this.leak != null) {
                this.leak.close();
            }
            return Collections.emptySet();
        }
        boolean interrupted = false;
        while (this.workerThread.isAlive()) {
            this.workerThread.interrupt();
            try {
                this.workerThread.join(100L);
            }
            catch (InterruptedException e2) {
                interrupted = true;
            }
        }
        if (interrupted) {
            Thread.currentThread().interrupt();
        }
        if (this.leak != null) {
            this.leak.close();
        }
        HashSet<HashedWheelTimeout> unprocessedTimeouts = new HashSet<HashedWheelTimeout>();
        for (Set<HashedWheelTimeout> bucket : this.wheel) {
            unprocessedTimeouts.addAll(bucket);
            bucket.clear();
        }
        return Collections.unmodifiableSet(unprocessedTimeouts);
    }

    @Override
    public Timeout newTimeout(TimerTask task, long delay, TimeUnit unit) {
        long currentTime = System.nanoTime();
        if (task == null) {
            throw new NullPointerException("task");
        }
        if (unit == null) {
            throw new NullPointerException("unit");
        }
        this.start();
        long delayInNanos = unit.toNanos(delay);
        HashedWheelTimeout timeout = new HashedWheelTimeout(task, currentTime + delayInNanos);
        this.scheduleTimeout(timeout, delayInNanos);
        return timeout;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    void scheduleTimeout(HashedWheelTimeout timeout, long delay) {
        long relativeIndex = (delay + this.tickDuration - 1L) / this.tickDuration;
        if (relativeIndex < 0L) {
            relativeIndex = delay / this.tickDuration;
        }
        if (relativeIndex == 0L) {
            relativeIndex = 1L;
        }
        if ((relativeIndex & (long)this.mask) == 0L) {
            --relativeIndex;
        }
        long remainingRounds = relativeIndex / (long)this.wheel.length;
        this.lock.readLock().lock();
        try {
            int stopIndex;
            if (this.workerState.get() == 2) {
                throw new IllegalStateException("Cannot enqueue after shutdown");
            }
            timeout.stopIndex = stopIndex = (int)((long)this.wheelCursor + relativeIndex & (long)this.mask);
            timeout.remainingRounds = remainingRounds;
            this.wheel[stopIndex].add(timeout);
        }
        finally {
            this.lock.readLock().unlock();
        }
    }

    private final class HashedWheelTimeout
    implements Timeout {
        private static final int ST_INIT = 0;
        private static final int ST_CANCELLED = 1;
        private static final int ST_EXPIRED = 2;
        private final TimerTask task;
        final long deadline;
        volatile int stopIndex;
        volatile long remainingRounds;
        private final AtomicInteger state = new AtomicInteger(0);

        HashedWheelTimeout(TimerTask task, long deadline) {
            this.task = task;
            this.deadline = deadline;
        }

        @Override
        public Timer timer() {
            return HashedWheelTimer.this;
        }

        @Override
        public TimerTask task() {
            return this.task;
        }

        @Override
        public boolean cancel() {
            if (!this.state.compareAndSet(0, 1)) {
                return false;
            }
            HashedWheelTimer.this.wheel[this.stopIndex].remove(this);
            return true;
        }

        @Override
        public boolean isCancelled() {
            return this.state.get() == 1;
        }

        @Override
        public boolean isExpired() {
            return this.state.get() != 0;
        }

        public void expire() {
            block3: {
                if (!this.state.compareAndSet(0, 2)) {
                    return;
                }
                try {
                    this.task.run(this);
                }
                catch (Throwable t2) {
                    if (!logger.isWarnEnabled()) break block3;
                    logger.warn("An exception was thrown by " + TimerTask.class.getSimpleName() + '.', t2);
                }
            }
        }

        public String toString() {
            long currentTime = System.nanoTime();
            long remaining = this.deadline - currentTime;
            StringBuilder buf = new StringBuilder(192);
            buf.append(this.getClass().getSimpleName());
            buf.append('(');
            buf.append("deadline: ");
            if (remaining > 0L) {
                buf.append(remaining);
                buf.append(" ms later, ");
            } else if (remaining < 0L) {
                buf.append(-remaining);
                buf.append(" ms ago, ");
            } else {
                buf.append("now, ");
            }
            if (this.isCancelled()) {
                buf.append(", cancelled");
            }
            return buf.append(')').toString();
        }
    }

    private final class Worker
    implements Runnable {
        private long startTime;
        private long tick;

        Worker() {
        }

        @Override
        public void run() {
            ArrayList<HashedWheelTimeout> expiredTimeouts = new ArrayList<HashedWheelTimeout>();
            this.startTime = System.nanoTime();
            this.tick = 1L;
            while (HashedWheelTimer.this.workerState.get() == 1) {
                long deadline = this.waitForNextTick();
                if (deadline <= 0L) continue;
                this.fetchExpiredTimeouts(expiredTimeouts, deadline);
                this.notifyExpiredTimeouts(expiredTimeouts);
            }
        }

        /*
         * WARNING - Removed try catching itself - possible behaviour change.
         */
        private void fetchExpiredTimeouts(List<HashedWheelTimeout> expiredTimeouts, long deadline) {
            HashedWheelTimer.this.lock.writeLock().lock();
            try {
                int newWheelCursor = HashedWheelTimer.this.wheelCursor = HashedWheelTimer.this.wheelCursor + 1 & HashedWheelTimer.this.mask;
                this.fetchExpiredTimeouts(expiredTimeouts, HashedWheelTimer.this.wheel[newWheelCursor].iterator(), deadline);
            }
            finally {
                HashedWheelTimer.this.lock.writeLock().unlock();
            }
        }

        private void fetchExpiredTimeouts(List<HashedWheelTimeout> expiredTimeouts, Iterator<HashedWheelTimeout> i2, long deadline) {
            ArrayList<HashedWheelTimeout> slipped = null;
            while (i2.hasNext()) {
                HashedWheelTimeout timeout = i2.next();
                if (timeout.remainingRounds <= 0L) {
                    i2.remove();
                    if (timeout.deadline <= deadline) {
                        expiredTimeouts.add(timeout);
                        continue;
                    }
                    if (slipped == null) {
                        slipped = new ArrayList<HashedWheelTimeout>();
                    }
                    slipped.add(timeout);
                    continue;
                }
                --timeout.remainingRounds;
            }
            if (slipped != null) {
                for (HashedWheelTimeout timeout : slipped) {
                    HashedWheelTimer.this.scheduleTimeout(timeout, timeout.deadline - deadline);
                }
            }
        }

        private void notifyExpiredTimeouts(List<HashedWheelTimeout> expiredTimeouts) {
            for (int i2 = expiredTimeouts.size() - 1; i2 >= 0; --i2) {
                expiredTimeouts.get(i2).expire();
            }
            expiredTimeouts.clear();
        }

        /*
         * Enabled force condition propagation
         * Lifted jumps to return sites
         */
        private long waitForNextTick() {
            long deadline = this.startTime + HashedWheelTimer.this.tickDuration * this.tick;
            while (true) {
                long currentTime;
                long sleepTimeMs;
                if ((sleepTimeMs = (deadline - (currentTime = System.nanoTime()) + 999999L) / 1000000L) <= 0L) {
                    ++this.tick;
                    if (currentTime != Long.MIN_VALUE) return currentTime;
                    return -9223372036854775807L;
                }
                if (PlatformDependent.isWindows()) {
                    sleepTimeMs = sleepTimeMs / 10L * 10L;
                }
                try {
                    Thread.sleep(sleepTimeMs);
                    continue;
                }
                catch (InterruptedException e2) {
                    if (HashedWheelTimer.this.workerState.get() == 2) return Long.MIN_VALUE;
                    continue;
                }
                break;
            }
        }
    }
}

