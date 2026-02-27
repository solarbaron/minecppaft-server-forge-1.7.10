/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util.internal.logging;

import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.JdkLoggerFactory;
import io.netty.util.internal.logging.Log4JLoggerFactory;
import io.netty.util.internal.logging.Slf4JLoggerFactory;

public abstract class InternalLoggerFactory {
    private static volatile InternalLoggerFactory defaultFactory;

    public static InternalLoggerFactory getDefaultFactory() {
        return defaultFactory;
    }

    public static void setDefaultFactory(InternalLoggerFactory defaultFactory) {
        if (defaultFactory == null) {
            throw new NullPointerException("defaultFactory");
        }
        InternalLoggerFactory.defaultFactory = defaultFactory;
    }

    public static InternalLogger getInstance(Class<?> clazz) {
        return InternalLoggerFactory.getInstance(clazz.getName());
    }

    public static InternalLogger getInstance(String name) {
        return InternalLoggerFactory.getDefaultFactory().newInstance(name);
    }

    protected abstract InternalLogger newInstance(String var1);

    static {
        InternalLoggerFactory f2;
        String name = InternalLoggerFactory.class.getName();
        try {
            f2 = new Slf4JLoggerFactory(true);
            f2.newInstance(name).debug("Using SLF4J as the default logging framework");
            defaultFactory = f2;
        }
        catch (Throwable t1) {
            try {
                f2 = new Log4JLoggerFactory();
                f2.newInstance(name).debug("Using Log4J as the default logging framework");
            }
            catch (Throwable t2) {
                f2 = new JdkLoggerFactory();
                f2.newInstance(name).debug("Using java.util.logging as the default logging framework");
            }
        }
        defaultFactory = f2;
    }
}

