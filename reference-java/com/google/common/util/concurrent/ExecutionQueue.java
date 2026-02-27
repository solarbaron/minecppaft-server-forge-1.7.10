/*
 * Decompiled with CFR 0.152.
 */
package com.google.common.util.concurrent;

import com.google.common.base.Preconditions;
import com.google.common.collect.Queues;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.Executor;
import java.util.concurrent.locks.ReentrantLock;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.concurrent.GuardedBy;
import javax.annotation.concurrent.ThreadSafe;

@ThreadSafe
final class ExecutionQueue {
    private static final Logger logger = Logger.getLogger(ExecutionQueue.class.getName());
    private final ConcurrentLinkedQueue<RunnableExecutorPair> queuedListeners = Queues.newConcurrentLinkedQueue();
    private final ReentrantLock lock = new ReentrantLock();

    ExecutionQueue() {
    }

    void add(Runnable runnable, Executor executor) {
        this.queuedListeners.add(new RunnableExecutorPair(runnable, executor));
    }

    void execute() {
        Iterator<RunnableExecutorPair> iterator = this.queuedListeners.iterator();
        while (iterator.hasNext()) {
            iterator.next().submit();
            iterator.remove();
        }
    }

    private final class RunnableExecutorPair
    implements Runnable {
        private final Executor executor;
        private final Runnable runnable;
        @GuardedBy(value="lock")
        private boolean hasBeenExecuted = false;

        RunnableExecutorPair(Runnable runnable, Executor executor) {
            this.runnable = Preconditions.checkNotNull(runnable);
            this.executor = Preconditions.checkNotNull(executor);
        }

        /*
         * WARNING - Removed try catching itself - possible behaviour change.
         */
        private void submit() {
            ExecutionQueue.this.lock.lock();
            try {
                if (!this.hasBeenExecuted) {
                    try {
                        this.executor.execute(this);
                    }
                    catch (Exception e2) {
                        logger.log(Level.SEVERE, "Exception while executing listener " + this.runnable + " with executor " + this.executor, e2);
                    }
                }
            }
            finally {
                if (ExecutionQueue.this.lock.isHeldByCurrentThread()) {
                    this.hasBeenExecuted = true;
                    ExecutionQueue.this.lock.unlock();
                }
            }
        }

        @Override
        public final void run() {
            if (ExecutionQueue.this.lock.isHeldByCurrentThread()) {
                this.hasBeenExecuted = true;
                ExecutionQueue.this.lock.unlock();
            }
            this.runnable.run();
        }
    }
}

