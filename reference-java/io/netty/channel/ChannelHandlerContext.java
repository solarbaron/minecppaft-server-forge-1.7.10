/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel;

import io.netty.channel.Channel;
import io.netty.channel.ChannelHandler;
import io.netty.channel.ChannelInboundInvoker;
import io.netty.channel.ChannelOutboundInvoker;
import io.netty.channel.ChannelPropertyAccess;
import io.netty.util.AttributeMap;
import io.netty.util.concurrent.EventExecutor;

public interface ChannelHandlerContext
extends AttributeMap,
ChannelPropertyAccess,
ChannelInboundInvoker,
ChannelOutboundInvoker {
    public Channel channel();

    public EventExecutor executor();

    public String name();

    public ChannelHandler handler();

    public boolean isRemoved();

    @Override
    public ChannelHandlerContext fireChannelRegistered();

    @Override
    @Deprecated
    public ChannelHandlerContext fireChannelUnregistered();

    @Override
    public ChannelHandlerContext fireChannelActive();

    @Override
    public ChannelHandlerContext fireChannelInactive();

    @Override
    public ChannelHandlerContext fireExceptionCaught(Throwable var1);

    @Override
    public ChannelHandlerContext fireUserEventTriggered(Object var1);

    @Override
    public ChannelHandlerContext fireChannelRead(Object var1);

    @Override
    public ChannelHandlerContext fireChannelReadComplete();

    @Override
    public ChannelHandlerContext fireChannelWritabilityChanged();

    @Override
    public ChannelHandlerContext flush();
}

