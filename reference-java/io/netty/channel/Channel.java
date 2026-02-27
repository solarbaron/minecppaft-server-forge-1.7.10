/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel;

import io.netty.channel.ChannelConfig;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelMetadata;
import io.netty.channel.ChannelOutboundBuffer;
import io.netty.channel.ChannelOutboundInvoker;
import io.netty.channel.ChannelPromise;
import io.netty.channel.ChannelPropertyAccess;
import io.netty.channel.EventLoop;
import io.netty.util.AttributeMap;
import java.net.SocketAddress;

public interface Channel
extends AttributeMap,
ChannelOutboundInvoker,
ChannelPropertyAccess,
Comparable<Channel> {
    public EventLoop eventLoop();

    public Channel parent();

    public ChannelConfig config();

    public boolean isOpen();

    public boolean isRegistered();

    public boolean isActive();

    public ChannelMetadata metadata();

    public SocketAddress localAddress();

    public SocketAddress remoteAddress();

    public ChannelFuture closeFuture();

    public boolean isWritable();

    @Override
    public Channel flush();

    @Override
    public Channel read();

    public Unsafe unsafe();

    public static interface Unsafe {
        public SocketAddress localAddress();

        public SocketAddress remoteAddress();

        public void register(EventLoop var1, ChannelPromise var2);

        public void bind(SocketAddress var1, ChannelPromise var2);

        public void connect(SocketAddress var1, SocketAddress var2, ChannelPromise var3);

        public void disconnect(ChannelPromise var1);

        public void close(ChannelPromise var1);

        public void closeForcibly();

        @Deprecated
        public void deregister(ChannelPromise var1);

        public void beginRead();

        public void write(Object var1, ChannelPromise var2);

        public void flush();

        public ChannelPromise voidPromise();

        public ChannelOutboundBuffer outboundBuffer();
    }
}

