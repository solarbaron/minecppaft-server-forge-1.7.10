/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel;

import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.ChannelProgressivePromise;
import io.netty.channel.ChannelPromise;

interface ChannelPropertyAccess {
    public ChannelPipeline pipeline();

    public ByteBufAllocator alloc();

    public ChannelPromise newPromise();

    public ChannelProgressivePromise newProgressivePromise();

    public ChannelFuture newSucceededFuture();

    public ChannelFuture newFailedFuture(Throwable var1);

    public ChannelPromise voidPromise();
}

