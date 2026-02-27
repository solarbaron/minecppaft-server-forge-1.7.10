/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel;

import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.MessageSizeEstimator;
import io.netty.channel.RecvByteBufAllocator;
import io.netty.util.UniqueName;
import io.netty.util.internal.PlatformDependent;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.concurrent.ConcurrentMap;

public class ChannelOption<T>
extends UniqueName {
    private static final ConcurrentMap<String, Boolean> names = PlatformDependent.newConcurrentHashMap();
    public static final ChannelOption<ByteBufAllocator> ALLOCATOR = new ChannelOption("ALLOCATOR");
    public static final ChannelOption<RecvByteBufAllocator> RCVBUF_ALLOCATOR = new ChannelOption("RCVBUF_ALLOCATOR");
    public static final ChannelOption<MessageSizeEstimator> MESSAGE_SIZE_ESTIMATOR = new ChannelOption("MESSAGE_SIZE_ESTIMATOR");
    public static final ChannelOption<Integer> CONNECT_TIMEOUT_MILLIS = new ChannelOption("CONNECT_TIMEOUT_MILLIS");
    public static final ChannelOption<Integer> MAX_MESSAGES_PER_READ = new ChannelOption("MAX_MESSAGES_PER_READ");
    public static final ChannelOption<Integer> WRITE_SPIN_COUNT = new ChannelOption("WRITE_SPIN_COUNT");
    public static final ChannelOption<Integer> WRITE_BUFFER_HIGH_WATER_MARK = new ChannelOption("WRITE_BUFFER_HIGH_WATER_MARK");
    public static final ChannelOption<Integer> WRITE_BUFFER_LOW_WATER_MARK = new ChannelOption("WRITE_BUFFER_LOW_WATER_MARK");
    public static final ChannelOption<Boolean> ALLOW_HALF_CLOSURE = new ChannelOption("ALLOW_HALF_CLOSURE");
    public static final ChannelOption<Boolean> AUTO_READ = new ChannelOption("AUTO_READ");
    public static final ChannelOption<Boolean> SO_BROADCAST = new ChannelOption("SO_BROADCAST");
    public static final ChannelOption<Boolean> SO_KEEPALIVE = new ChannelOption("SO_KEEPALIVE");
    public static final ChannelOption<Integer> SO_SNDBUF = new ChannelOption("SO_SNDBUF");
    public static final ChannelOption<Integer> SO_RCVBUF = new ChannelOption("SO_RCVBUF");
    public static final ChannelOption<Boolean> SO_REUSEADDR = new ChannelOption("SO_REUSEADDR");
    public static final ChannelOption<Integer> SO_LINGER = new ChannelOption("SO_LINGER");
    public static final ChannelOption<Integer> SO_BACKLOG = new ChannelOption("SO_BACKLOG");
    public static final ChannelOption<Integer> SO_TIMEOUT = new ChannelOption("SO_TIMEOUT");
    public static final ChannelOption<Integer> IP_TOS = new ChannelOption("IP_TOS");
    public static final ChannelOption<InetAddress> IP_MULTICAST_ADDR = new ChannelOption("IP_MULTICAST_ADDR");
    public static final ChannelOption<NetworkInterface> IP_MULTICAST_IF = new ChannelOption("IP_MULTICAST_IF");
    public static final ChannelOption<Integer> IP_MULTICAST_TTL = new ChannelOption("IP_MULTICAST_TTL");
    public static final ChannelOption<Boolean> IP_MULTICAST_LOOP_DISABLED = new ChannelOption("IP_MULTICAST_LOOP_DISABLED");
    public static final ChannelOption<Boolean> TCP_NODELAY = new ChannelOption("TCP_NODELAY");
    @Deprecated
    public static final ChannelOption<Long> AIO_READ_TIMEOUT = new ChannelOption("AIO_READ_TIMEOUT");
    @Deprecated
    public static final ChannelOption<Long> AIO_WRITE_TIMEOUT = new ChannelOption("AIO_WRITE_TIMEOUT");
    @Deprecated
    public static final ChannelOption<Boolean> DATAGRAM_CHANNEL_ACTIVE_ON_REGISTRATION = new ChannelOption("DATAGRAM_CHANNEL_ACTIVE_ON_REGISTRATION");

    protected ChannelOption(String name) {
        super(names, name, new Object[0]);
    }

    public void validate(T value) {
        if (value == null) {
            throw new NullPointerException("value");
        }
    }
}

