/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util;

import io.netty.util.ResourceLeak;
import io.netty.util.ResourceLeakException;
import io.netty.util.internal.PlatformDependent;
import io.netty.util.internal.SystemPropertyUtil;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicBoolean;

public final class ResourceLeakDetector<T> {
    private static boolean disabled;
    private static final InternalLogger logger;
    private static final int DEFAULT_SAMPLING_INTERVAL = 113;
    private final DefaultResourceLeak head = new DefaultResourceLeak((Object)null);
    private final DefaultResourceLeak tail = new DefaultResourceLeak((Object)null);
    private final ReferenceQueue<Object> refQueue = new ReferenceQueue();
    private final ConcurrentMap<Exception, Boolean> reportedLeaks = PlatformDependent.newConcurrentHashMap();
    private final String resourceType;
    private final int samplingInterval;
    private final long maxActive;
    private long active;
    private final AtomicBoolean loggedTooManyActive = new AtomicBoolean();
    private long leakCheckCnt;

    public static void setEnabled(boolean enabled) {
        disabled = !enabled;
    }

    public static boolean isEnabled() {
        return !disabled;
    }

    public ResourceLeakDetector(Class<?> resourceType) {
        this(resourceType.getSimpleName());
    }

    public ResourceLeakDetector(String resourceType) {
        this(resourceType, 113, Long.MAX_VALUE);
    }

    public ResourceLeakDetector(Class<?> resourceType, int samplingInterval, long maxActive) {
        this(resourceType.getSimpleName(), samplingInterval, maxActive);
    }

    public ResourceLeakDetector(String resourceType, int samplingInterval, long maxActive) {
        if (resourceType == null) {
            throw new NullPointerException("resourceType");
        }
        if (samplingInterval <= 0) {
            throw new IllegalArgumentException("samplingInterval: " + samplingInterval + " (expected: 1+)");
        }
        if (maxActive <= 0L) {
            throw new IllegalArgumentException("maxActive: " + maxActive + " (expected: 1+)");
        }
        this.resourceType = resourceType;
        this.samplingInterval = samplingInterval;
        this.maxActive = maxActive;
        this.head.next = this.tail;
        this.tail.prev = this.head;
    }

    public ResourceLeak open(T obj) {
        if (disabled || this.leakCheckCnt++ % (long)this.samplingInterval != 0L) {
            return null;
        }
        this.reportLeak();
        return new DefaultResourceLeak(obj);
    }

    private void reportLeak() {
        DefaultResourceLeak ref;
        if (!logger.isWarnEnabled()) {
            DefaultResourceLeak ref2;
            while ((ref2 = (DefaultResourceLeak)this.refQueue.poll()) != null) {
                ref2.close();
            }
            return;
        }
        if (this.active * (long)this.samplingInterval > this.maxActive && this.loggedTooManyActive.compareAndSet(false, true)) {
            logger.warn("LEAK: You are creating too many " + this.resourceType + " instances.  " + this.resourceType + " is a shared resource that must be reused across the JVM," + "so that only a few instances are created.");
        }
        while ((ref = (DefaultResourceLeak)this.refQueue.poll()) != null) {
            ref.clear();
            if (!ref.close() || this.reportedLeaks.putIfAbsent(ref.exception, Boolean.TRUE) != null) continue;
            logger.warn("LEAK: " + this.resourceType + " was GC'd before being released correctly.  " + "The following stack trace shows where the leaked object was created, " + "rather than where you failed to release it.", ref.exception);
        }
    }

    static {
        logger = InternalLoggerFactory.getInstance(ResourceLeakDetector.class);
        boolean DISABLED = SystemPropertyUtil.getBoolean("io.netty.noResourceLeakDetection", false);
        logger.debug("-Dio.netty.noResourceLeakDetection: {}", (Object)DISABLED);
        disabled = DISABLED;
    }

    private final class DefaultResourceLeak
    extends PhantomReference<Object>
    implements ResourceLeak {
        private final ResourceLeakException exception;
        private final AtomicBoolean freed;
        private DefaultResourceLeak prev;
        private DefaultResourceLeak next;

        /*
         * WARNING - Removed try catching itself - possible behaviour change.
         */
        public DefaultResourceLeak(Object referent) {
            super(referent, referent != null ? ResourceLeakDetector.this.refQueue : null);
            if (referent != null) {
                this.exception = new ResourceLeakException(referent.getClass().getName() + '@' + Integer.toHexString(System.identityHashCode(referent)));
                DefaultResourceLeak defaultResourceLeak = ResourceLeakDetector.this.head;
                synchronized (defaultResourceLeak) {
                    this.prev = ResourceLeakDetector.this.head;
                    this.next = ((ResourceLeakDetector)ResourceLeakDetector.this).head.next;
                    ((ResourceLeakDetector)ResourceLeakDetector.this).head.next.prev = this;
                    ((ResourceLeakDetector)ResourceLeakDetector.this).head.next = this;
                    ResourceLeakDetector.this.active++;
                }
                this.freed = new AtomicBoolean();
            } else {
                this.exception = null;
                this.freed = new AtomicBoolean(true);
            }
        }

        /*
         * WARNING - Removed try catching itself - possible behaviour change.
         */
        @Override
        public boolean close() {
            if (this.freed.compareAndSet(false, true)) {
                DefaultResourceLeak defaultResourceLeak = ResourceLeakDetector.this.head;
                synchronized (defaultResourceLeak) {
                    ResourceLeakDetector.this.active--;
                    this.prev.next = this.next;
                    this.next.prev = this.prev;
                    this.prev = null;
                    this.next = null;
                }
                return true;
            }
            return false;
        }
    }
}

