/*
 * Decompiled with CFR 0.152.
 */
package io.netty.util;

import io.netty.util.UniqueName;
import io.netty.util.internal.PlatformDependent;
import java.util.concurrent.ConcurrentMap;

public final class AttributeKey<T>
extends UniqueName {
    private static final ConcurrentMap<String, Boolean> names = PlatformDependent.newConcurrentHashMap();

    public AttributeKey(String name) {
        super(names, name, new Object[0]);
    }
}

