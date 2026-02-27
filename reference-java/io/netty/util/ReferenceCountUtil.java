/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util;

import io.netty.util.ReferenceCounted;

public final class ReferenceCountUtil {
    public static <T> T retain(T msg) {
        if (msg instanceof ReferenceCounted) {
            return (T)((ReferenceCounted)msg).retain();
        }
        return msg;
    }

    public static <T> T retain(T msg, int increment) {
        if (msg instanceof ReferenceCounted) {
            return (T)((ReferenceCounted)msg).retain(increment);
        }
        return msg;
    }

    public static boolean release(Object msg) {
        if (msg instanceof ReferenceCounted) {
            return ((ReferenceCounted)msg).release();
        }
        return false;
    }

    public static boolean release(Object msg, int decrement) {
        if (msg instanceof ReferenceCounted) {
            return ((ReferenceCounted)msg).release(decrement);
        }
        return false;
    }

    private ReferenceCountUtil() {
    }
}

