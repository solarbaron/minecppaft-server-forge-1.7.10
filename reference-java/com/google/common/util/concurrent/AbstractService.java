/*
 * Decompiled with CFR 0.152.
 */
package com.google.common.util.concurrent;

import com.google.common.annotations.Beta;
import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.google.common.util.concurrent.AbstractFuture;
import com.google.common.util.concurrent.ExecutionQueue;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.Monitor;
import com.google.common.util.concurrent.MoreExecutors;
import com.google.common.util.concurrent.Service;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import javax.annotation.Nullable;
import javax.annotation.concurrent.GuardedBy;
import javax.annotation.concurrent.Immutable;

@Beta
public abstract class AbstractService
implements Service {
    private final Monitor monitor = new Monitor();
    private final Transition startup = new Transition();
    private final Transition shutdown = new Transition();
    private final Monitor.Guard isStartable = new Monitor.Guard(this.monitor){

        @Override
        public boolean isSatisfied() {
            return AbstractService.this.state() == Service.State.NEW;
        }
    };
    private final Monitor.Guard isStoppable = new Monitor.Guard(this.monitor){

        @Override
        public boolean isSatisfied() {
            return AbstractService.this.state().compareTo(Service.State.RUNNING) <= 0;
        }
    };
    private final Monitor.Guard hasReachedRunning = new Monitor.Guard(this.monitor){

        @Override
        public boolean isSatisfied() {
            return AbstractService.this.state().compareTo(Service.State.RUNNING) >= 0;
        }
    };
    private final Monitor.Guard isStopped = new Monitor.Guard(this.monitor){

        @Override
        public boolean isSatisfied() {
            return AbstractService.this.state().isTerminal();
        }
    };
    @GuardedBy(value="monitor")
    private final List<ListenerExecutorPair> listeners = Lists.newArrayList();
    private final ExecutionQueue queuedListeners = new ExecutionQueue();
    @GuardedBy(value="monitor")
    private volatile StateSnapshot snapshot = new StateSnapshot(Service.State.NEW);

    protected AbstractService() {
        this.addListener(new Service.Listener(){

            @Override
            public void running() {
                AbstractService.this.startup.set(Service.State.RUNNING);
            }

            @Override
            public void stopping(Service.State from) {
                if (from == Service.State.STARTING) {
                    AbstractService.this.startup.set(Service.State.STOPPING);
                }
            }

            @Override
            public void terminated(Service.State from) {
                if (from == Service.State.NEW) {
                    AbstractService.this.startup.set(Service.State.TERMINATED);
                }
                AbstractService.this.shutdown.set(Service.State.TERMINATED);
            }

            @Override
            public void failed(Service.State from, Throwable failure) {
                switch (from) {
                    case STARTING: {
                        AbstractService.this.startup.setException(failure);
                        AbstractService.this.shutdown.setException(new Exception("Service failed to start.", failure));
                        break;
                    }
                    case RUNNING: {
                        AbstractService.this.shutdown.setException(new Exception("Service failed while running", failure));
                        break;
                    }
                    case STOPPING: {
                        AbstractService.this.shutdown.setException(failure);
                        break;
                    }
                    default: {
                        throw new AssertionError((Object)("Unexpected from state: " + (Object)((Object)from)));
                    }
                }
            }
        }, MoreExecutors.sameThreadExecutor());
    }

    protected abstract void doStart();

    protected abstract void doStop();

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final Service startAsync() {
        if (this.monitor.enterIf(this.isStartable)) {
            try {
                this.snapshot = new StateSnapshot(Service.State.STARTING);
                this.starting();
                this.doStart();
            }
            catch (Throwable startupFailure) {
                this.notifyFailed(startupFailure);
            }
            finally {
                this.monitor.leave();
                this.executeListeners();
            }
        } else {
            throw new IllegalStateException("Service " + this + " has already been started");
        }
        return this;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    @Deprecated
    public final ListenableFuture<Service.State> start() {
        if (this.monitor.enterIf(this.isStartable)) {
            try {
                this.snapshot = new StateSnapshot(Service.State.STARTING);
                this.starting();
                this.doStart();
            }
            catch (Throwable startupFailure) {
                this.notifyFailed(startupFailure);
            }
            finally {
                this.monitor.leave();
                this.executeListeners();
            }
        }
        return this.startup;
    }

    @Override
    public final Service stopAsync() {
        this.stop();
        return this;
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    @Override
    @Deprecated
    public final ListenableFuture<Service.State> stop() {
        if (!this.monitor.enterIf(this.isStoppable)) return this.shutdown;
        try {
            Service.State previous = this.state();
            switch (previous) {
                case NEW: {
                    this.snapshot = new StateSnapshot(Service.State.TERMINATED);
                    this.terminated(Service.State.NEW);
                    return this.shutdown;
                }
                case STARTING: {
                    this.snapshot = new StateSnapshot(Service.State.STARTING, true, null);
                    this.stopping(Service.State.STARTING);
                    return this.shutdown;
                }
                case RUNNING: {
                    this.snapshot = new StateSnapshot(Service.State.STOPPING);
                    this.stopping(Service.State.RUNNING);
                    this.doStop();
                    return this.shutdown;
                }
                case STOPPING: 
                case TERMINATED: 
                case FAILED: {
                    throw new AssertionError((Object)("isStoppable is incorrectly implemented, saw: " + (Object)((Object)previous)));
                }
                default: {
                    throw new AssertionError((Object)("Unexpected state: " + (Object)((Object)previous)));
                }
            }
        }
        catch (Throwable shutdownFailure) {
            this.notifyFailed(shutdownFailure);
            return this.shutdown;
        }
        finally {
            this.monitor.leave();
            this.executeListeners();
        }
    }

    @Override
    @Deprecated
    public Service.State startAndWait() {
        return Futures.getUnchecked(this.start());
    }

    @Override
    @Deprecated
    public Service.State stopAndWait() {
        return Futures.getUnchecked(this.stop());
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final void awaitRunning() {
        this.monitor.enterWhenUninterruptibly(this.hasReachedRunning);
        try {
            this.checkCurrentState(Service.State.RUNNING);
        }
        finally {
            this.monitor.leave();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final void awaitRunning(long timeout, TimeUnit unit) throws TimeoutException {
        if (this.monitor.enterWhenUninterruptibly(this.hasReachedRunning, timeout, unit)) {
            try {
                this.checkCurrentState(Service.State.RUNNING);
            }
            finally {
                this.monitor.leave();
            }
        } else {
            throw new TimeoutException("Timed out waiting for " + this + " to reach the RUNNING state. " + "Current state: " + (Object)((Object)this.state()));
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final void awaitTerminated() {
        this.monitor.enterWhenUninterruptibly(this.isStopped);
        try {
            this.checkCurrentState(Service.State.TERMINATED);
        }
        finally {
            this.monitor.leave();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final void awaitTerminated(long timeout, TimeUnit unit) throws TimeoutException {
        if (this.monitor.enterWhenUninterruptibly(this.isStopped, timeout, unit)) {
            try {
                Service.State state = this.state();
                this.checkCurrentState(Service.State.TERMINATED);
            }
            finally {
                this.monitor.leave();
            }
        } else {
            throw new TimeoutException("Timed out waiting for " + this + " to reach a terminal state. " + "Current state: " + (Object)((Object)this.state()));
        }
    }

    @GuardedBy(value="monitor")
    private void checkCurrentState(Service.State expected) {
        Service.State actual = this.state();
        if (actual != expected) {
            if (actual == Service.State.FAILED) {
                throw new IllegalStateException("Expected the service to be " + (Object)((Object)expected) + ", but the service has FAILED", this.failureCause());
            }
            throw new IllegalStateException("Expected the service to be " + (Object)((Object)expected) + ", but was " + (Object)((Object)actual));
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    protected final void notifyStarted() {
        this.monitor.enter();
        try {
            if (this.snapshot.state != Service.State.STARTING) {
                IllegalStateException failure = new IllegalStateException("Cannot notifyStarted() when the service is " + (Object)((Object)this.snapshot.state));
                this.notifyFailed(failure);
                throw failure;
            }
            if (this.snapshot.shutdownWhenStartupFinishes) {
                this.snapshot = new StateSnapshot(Service.State.STOPPING);
                this.doStop();
            } else {
                this.snapshot = new StateSnapshot(Service.State.RUNNING);
                this.running();
            }
        }
        finally {
            this.monitor.leave();
            this.executeListeners();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    protected final void notifyStopped() {
        this.monitor.enter();
        try {
            Service.State previous = this.snapshot.state;
            if (previous != Service.State.STOPPING && previous != Service.State.RUNNING) {
                IllegalStateException failure = new IllegalStateException("Cannot notifyStopped() when the service is " + (Object)((Object)previous));
                this.notifyFailed(failure);
                throw failure;
            }
            this.snapshot = new StateSnapshot(Service.State.TERMINATED);
            this.terminated(previous);
        }
        finally {
            this.monitor.leave();
            this.executeListeners();
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    protected final void notifyFailed(Throwable cause) {
        Preconditions.checkNotNull(cause);
        this.monitor.enter();
        try {
            Service.State previous = this.state();
            switch (previous) {
                case TERMINATED: 
                case NEW: {
                    throw new IllegalStateException("Failed while in state:" + (Object)((Object)previous), cause);
                }
                case STARTING: 
                case RUNNING: 
                case STOPPING: {
                    this.snapshot = new StateSnapshot(Service.State.FAILED, false, cause);
                    this.failed(previous, cause);
                    return;
                }
                case FAILED: {
                    return;
                }
                default: {
                    throw new AssertionError((Object)("Unexpected state: " + (Object)((Object)previous)));
                }
            }
        }
        finally {
            this.monitor.leave();
            this.executeListeners();
        }
    }

    @Override
    public final boolean isRunning() {
        return this.state() == Service.State.RUNNING;
    }

    @Override
    public final Service.State state() {
        return this.snapshot.externalState();
    }

    @Override
    public final Throwable failureCause() {
        return this.snapshot.failureCause();
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public final void addListener(Service.Listener listener, Executor executor) {
        Preconditions.checkNotNull(listener, "listener");
        Preconditions.checkNotNull(executor, "executor");
        this.monitor.enter();
        try {
            Service.State currentState = this.state();
            if (currentState != Service.State.TERMINATED && currentState != Service.State.FAILED) {
                this.listeners.add(new ListenerExecutorPair(listener, executor));
            }
        }
        finally {
            this.monitor.leave();
        }
    }

    public String toString() {
        return this.getClass().getSimpleName() + " [" + (Object)((Object)this.state()) + "]";
    }

    private void executeListeners() {
        if (!this.monitor.isOccupiedByCurrentThread()) {
            this.queuedListeners.execute();
        }
    }

    @GuardedBy(value="monitor")
    private void starting() {
        for (final ListenerExecutorPair pair : this.listeners) {
            this.queuedListeners.add(new Runnable(){

                @Override
                public void run() {
                    pair.listener.starting();
                }
            }, pair.executor);
        }
    }

    @GuardedBy(value="monitor")
    private void running() {
        for (final ListenerExecutorPair pair : this.listeners) {
            this.queuedListeners.add(new Runnable(){

                @Override
                public void run() {
                    pair.listener.running();
                }
            }, pair.executor);
        }
    }

    @GuardedBy(value="monitor")
    private void stopping(final Service.State from) {
        for (final ListenerExecutorPair pair : this.listeners) {
            this.queuedListeners.add(new Runnable(){

                @Override
                public void run() {
                    pair.listener.stopping(from);
                }
            }, pair.executor);
        }
    }

    @GuardedBy(value="monitor")
    private void terminated(final Service.State from) {
        for (final ListenerExecutorPair pair : this.listeners) {
            this.queuedListeners.add(new Runnable(){

                @Override
                public void run() {
                    pair.listener.terminated(from);
                }
            }, pair.executor);
        }
        this.listeners.clear();
    }

    @GuardedBy(value="monitor")
    private void failed(final Service.State from, final Throwable cause) {
        for (final ListenerExecutorPair pair : this.listeners) {
            this.queuedListeners.add(new Runnable(){

                @Override
                public void run() {
                    pair.listener.failed(from, cause);
                }
            }, pair.executor);
        }
        this.listeners.clear();
    }

    @Immutable
    private static final class StateSnapshot {
        final Service.State state;
        final boolean shutdownWhenStartupFinishes;
        @Nullable
        final Throwable failure;

        StateSnapshot(Service.State internalState) {
            this(internalState, false, null);
        }

        StateSnapshot(Service.State internalState, boolean shutdownWhenStartupFinishes, @Nullable Throwable failure) {
            Preconditions.checkArgument(!shutdownWhenStartupFinishes || internalState == Service.State.STARTING, "shudownWhenStartupFinishes can only be set if state is STARTING. Got %s instead.", new Object[]{internalState});
            Preconditions.checkArgument(!(failure != null ^ internalState == Service.State.FAILED), "A failure cause should be set if and only if the state is failed.  Got %s and %s instead.", new Object[]{internalState, failure});
            this.state = internalState;
            this.shutdownWhenStartupFinishes = shutdownWhenStartupFinishes;
            this.failure = failure;
        }

        Service.State externalState() {
            if (this.shutdownWhenStartupFinishes && this.state == Service.State.STARTING) {
                return Service.State.STOPPING;
            }
            return this.state;
        }

        Throwable failureCause() {
            Preconditions.checkState(this.state == Service.State.FAILED, "failureCause() is only valid if the service has failed, service is %s", new Object[]{this.state});
            return this.failure;
        }
    }

    private static class ListenerExecutorPair {
        final Service.Listener listener;
        final Executor executor;

        ListenerExecutorPair(Service.Listener listener, Executor executor) {
            this.listener = listener;
            this.executor = executor;
        }
    }

    private class Transition
    extends AbstractFuture<Service.State> {
        private Transition() {
        }

        @Override
        public Service.State get(long timeout, TimeUnit unit) throws InterruptedException, TimeoutException, ExecutionException {
            try {
                return (Service.State)((Object)super.get(timeout, unit));
            }
            catch (TimeoutException e2) {
                throw new TimeoutException(AbstractService.this.toString());
            }
        }
    }
}

