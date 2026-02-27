/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.socket.nio;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.ByteBufHolder;
import io.netty.channel.AddressedEnvelope;
import io.netty.channel.ChannelException;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelMetadata;
import io.netty.channel.ChannelOption;
import io.netty.channel.ChannelOutboundBuffer;
import io.netty.channel.ChannelPromise;
import io.netty.channel.DefaultAddressedEnvelope;
import io.netty.channel.RecvByteBufAllocator;
import io.netty.channel.nio.AbstractNioMessageChannel;
import io.netty.channel.socket.DatagramChannel;
import io.netty.channel.socket.DatagramChannelConfig;
import io.netty.channel.socket.DatagramPacket;
import io.netty.channel.socket.InternetProtocolFamily;
import io.netty.channel.socket.nio.NioDatagramChannelConfig;
import io.netty.channel.socket.nio.ProtocolFamilyConverter;
import io.netty.util.internal.PlatformDependent;
import io.netty.util.internal.StringUtil;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.MembershipKey;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

public final class NioDatagramChannel
extends AbstractNioMessageChannel
implements DatagramChannel {
    private static final ChannelMetadata METADATA = new ChannelMetadata(true);
    private final DatagramChannelConfig config;
    private final Map<InetAddress, List<MembershipKey>> memberships = new HashMap<InetAddress, List<MembershipKey>>();
    private RecvByteBufAllocator.Handle allocHandle;

    private static java.nio.channels.DatagramChannel newSocket() {
        try {
            return java.nio.channels.DatagramChannel.open();
        }
        catch (IOException e2) {
            throw new ChannelException("Failed to open a socket.", e2);
        }
    }

    private static java.nio.channels.DatagramChannel newSocket(InternetProtocolFamily ipFamily) {
        if (ipFamily == null) {
            return NioDatagramChannel.newSocket();
        }
        if (PlatformDependent.javaVersion() < 7) {
            throw new UnsupportedOperationException();
        }
        try {
            return java.nio.channels.DatagramChannel.open(ProtocolFamilyConverter.convert(ipFamily));
        }
        catch (IOException e2) {
            throw new ChannelException("Failed to open a socket.", e2);
        }
    }

    public NioDatagramChannel() {
        this(NioDatagramChannel.newSocket());
    }

    public NioDatagramChannel(InternetProtocolFamily ipFamily) {
        this(NioDatagramChannel.newSocket(ipFamily));
    }

    public NioDatagramChannel(java.nio.channels.DatagramChannel socket) {
        super(null, socket, 1);
        this.config = new NioDatagramChannelConfig(this, socket);
    }

    @Override
    public ChannelMetadata metadata() {
        return METADATA;
    }

    @Override
    public DatagramChannelConfig config() {
        return this.config;
    }

    @Override
    public boolean isActive() {
        java.nio.channels.DatagramChannel ch2 = this.javaChannel();
        return ch2.isOpen() && (this.config.getOption(ChannelOption.DATAGRAM_CHANNEL_ACTIVE_ON_REGISTRATION) != false && this.isRegistered() || ch2.socket().isBound());
    }

    @Override
    public boolean isConnected() {
        return this.javaChannel().isConnected();
    }

    @Override
    protected java.nio.channels.DatagramChannel javaChannel() {
        return (java.nio.channels.DatagramChannel)super.javaChannel();
    }

    @Override
    protected SocketAddress localAddress0() {
        return this.javaChannel().socket().getLocalSocketAddress();
    }

    @Override
    protected SocketAddress remoteAddress0() {
        return this.javaChannel().socket().getRemoteSocketAddress();
    }

    @Override
    protected void doBind(SocketAddress localAddress) throws Exception {
        this.javaChannel().socket().bind(localAddress);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    protected boolean doConnect(SocketAddress remoteAddress, SocketAddress localAddress) throws Exception {
        if (localAddress != null) {
            this.javaChannel().socket().bind(localAddress);
        }
        boolean success = false;
        try {
            this.javaChannel().connect(remoteAddress);
            success = true;
            boolean bl2 = true;
            return bl2;
        }
        finally {
            if (!success) {
                this.doClose();
            }
        }
    }

    @Override
    protected void doFinishConnect() throws Exception {
        throw new Error();
    }

    @Override
    protected void doDisconnect() throws Exception {
        this.javaChannel().disconnect();
    }

    @Override
    protected void doClose() throws Exception {
        this.javaChannel().close();
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    protected int doReadMessages(List<Object> buf) throws Exception {
        java.nio.channels.DatagramChannel ch2 = this.javaChannel();
        DatagramChannelConfig config = this.config();
        RecvByteBufAllocator.Handle allocHandle = this.allocHandle;
        if (allocHandle == null) {
            this.allocHandle = allocHandle = config.getRecvByteBufAllocator().newHandle();
        }
        ByteBuf data = allocHandle.allocate(config.getAllocator());
        boolean free = true;
        try {
            ByteBuffer nioData = data.nioBuffer(data.writerIndex(), data.writableBytes());
            InetSocketAddress remoteAddress = (InetSocketAddress)ch2.receive(nioData);
            if (remoteAddress == null) {
                int n2 = 0;
                return n2;
            }
            int readBytes = nioData.position();
            data.writerIndex(data.writerIndex() + readBytes);
            allocHandle.record(readBytes);
            buf.add(new DatagramPacket(data, this.localAddress(), remoteAddress));
            free = false;
            int n3 = 1;
            return n3;
        }
        catch (Throwable cause) {
            PlatformDependent.throwException(cause);
            int n4 = -1;
            return n4;
        }
        finally {
            if (free) {
                data.release();
            }
        }
    }

    @Override
    protected boolean doWriteMessage(Object msg, ChannelOutboundBuffer in2) throws Exception {
        boolean done;
        ByteBuffer nioData;
        boolean needsCopy;
        ByteBuf data;
        Object m2;
        SocketAddress remoteAddress;
        if (msg instanceof AddressedEnvelope) {
            AddressedEnvelope envelope = (AddressedEnvelope)msg;
            remoteAddress = (SocketAddress)envelope.recipient();
            m2 = envelope.content();
        } else {
            m2 = msg;
            remoteAddress = null;
        }
        if (m2 instanceof ByteBufHolder) {
            data = ((ByteBufHolder)m2).content();
        } else if (m2 instanceof ByteBuf) {
            data = (ByteBuf)m2;
        } else {
            throw new UnsupportedOperationException("unsupported message type: " + StringUtil.simpleClassName(msg));
        }
        int dataLen = data.readableBytes();
        if (dataLen == 0) {
            return true;
        }
        ByteBufAllocator alloc = this.alloc();
        boolean bl2 = needsCopy = data.nioBufferCount() != 1;
        if (!needsCopy && !data.isDirect() && alloc.isDirectBufferPooled()) {
            needsCopy = true;
        }
        if (!needsCopy) {
            nioData = data.nioBuffer();
        } else {
            data = alloc.directBuffer(dataLen).writeBytes(data);
            nioData = data.nioBuffer();
        }
        int writtenBytes = remoteAddress != null ? this.javaChannel().send(nioData, remoteAddress) : this.javaChannel().write(nioData);
        boolean bl3 = done = writtenBytes > 0;
        if (needsCopy) {
            if (remoteAddress == null) {
                in2.current(data);
            } else if (!done) {
                in2.current(new DefaultAddressedEnvelope<ByteBuf, Object>(data, remoteAddress));
            } else {
                in2.current(data);
            }
        }
        return done;
    }

    @Override
    public InetSocketAddress localAddress() {
        return (InetSocketAddress)super.localAddress();
    }

    @Override
    public InetSocketAddress remoteAddress() {
        return (InetSocketAddress)super.remoteAddress();
    }

    @Override
    public ChannelFuture joinGroup(InetAddress multicastAddress) {
        return this.joinGroup(multicastAddress, this.newPromise());
    }

    @Override
    public ChannelFuture joinGroup(InetAddress multicastAddress, ChannelPromise promise) {
        try {
            return this.joinGroup(multicastAddress, NetworkInterface.getByInetAddress(this.localAddress().getAddress()), null, promise);
        }
        catch (SocketException e2) {
            promise.setFailure(e2);
            return promise;
        }
    }

    @Override
    public ChannelFuture joinGroup(InetSocketAddress multicastAddress, NetworkInterface networkInterface) {
        return this.joinGroup(multicastAddress, networkInterface, this.newPromise());
    }

    @Override
    public ChannelFuture joinGroup(InetSocketAddress multicastAddress, NetworkInterface networkInterface, ChannelPromise promise) {
        return this.joinGroup(multicastAddress.getAddress(), networkInterface, null, promise);
    }

    @Override
    public ChannelFuture joinGroup(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress source) {
        return this.joinGroup(multicastAddress, networkInterface, source, this.newPromise());
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public ChannelFuture joinGroup(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress source, ChannelPromise promise) {
        if (PlatformDependent.javaVersion() >= 7) {
            if (multicastAddress == null) {
                throw new NullPointerException("multicastAddress");
            }
            if (networkInterface == null) {
                throw new NullPointerException("networkInterface");
            }
            try {
                MembershipKey key = source == null ? this.javaChannel().join(multicastAddress, networkInterface) : this.javaChannel().join(multicastAddress, networkInterface, source);
                NioDatagramChannel nioDatagramChannel = this;
                synchronized (nioDatagramChannel) {
                    List<MembershipKey> keys = this.memberships.get(multicastAddress);
                    if (keys == null) {
                        keys = new ArrayList<MembershipKey>();
                        this.memberships.put(multicastAddress, keys);
                    }
                    keys.add(key);
                }
                promise.setSuccess();
            }
            catch (Throwable e2) {
                promise.setFailure(e2);
            }
        } else {
            throw new UnsupportedOperationException();
        }
        return promise;
    }

    @Override
    public ChannelFuture leaveGroup(InetAddress multicastAddress) {
        return this.leaveGroup(multicastAddress, this.newPromise());
    }

    @Override
    public ChannelFuture leaveGroup(InetAddress multicastAddress, ChannelPromise promise) {
        try {
            return this.leaveGroup(multicastAddress, NetworkInterface.getByInetAddress(this.localAddress().getAddress()), null, promise);
        }
        catch (SocketException e2) {
            promise.setFailure(e2);
            return promise;
        }
    }

    @Override
    public ChannelFuture leaveGroup(InetSocketAddress multicastAddress, NetworkInterface networkInterface) {
        return this.leaveGroup(multicastAddress, networkInterface, this.newPromise());
    }

    @Override
    public ChannelFuture leaveGroup(InetSocketAddress multicastAddress, NetworkInterface networkInterface, ChannelPromise promise) {
        return this.leaveGroup(multicastAddress.getAddress(), networkInterface, null, promise);
    }

    @Override
    public ChannelFuture leaveGroup(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress source) {
        return this.leaveGroup(multicastAddress, networkInterface, source, this.newPromise());
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public ChannelFuture leaveGroup(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress source, ChannelPromise promise) {
        if (PlatformDependent.javaVersion() < 7) {
            throw new UnsupportedOperationException();
        }
        if (multicastAddress == null) {
            throw new NullPointerException("multicastAddress");
        }
        if (networkInterface == null) {
            throw new NullPointerException("networkInterface");
        }
        NioDatagramChannel nioDatagramChannel = this;
        synchronized (nioDatagramChannel) {
            List<MembershipKey> keys;
            if (this.memberships != null && (keys = this.memberships.get(multicastAddress)) != null) {
                Iterator<MembershipKey> keyIt = keys.iterator();
                while (keyIt.hasNext()) {
                    MembershipKey key = keyIt.next();
                    if (!networkInterface.equals(key.networkInterface()) || (source != null || key.sourceAddress() != null) && (source == null || !source.equals(key.sourceAddress()))) continue;
                    key.drop();
                    keyIt.remove();
                }
                if (keys.isEmpty()) {
                    this.memberships.remove(multicastAddress);
                }
            }
        }
        promise.setSuccess();
        return promise;
    }

    @Override
    public ChannelFuture block(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress sourceToBlock) {
        return this.block(multicastAddress, networkInterface, sourceToBlock, this.newPromise());
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public ChannelFuture block(InetAddress multicastAddress, NetworkInterface networkInterface, InetAddress sourceToBlock, ChannelPromise promise) {
        if (PlatformDependent.javaVersion() < 7) {
            throw new UnsupportedOperationException();
        }
        if (multicastAddress == null) {
            throw new NullPointerException("multicastAddress");
        }
        if (sourceToBlock == null) {
            throw new NullPointerException("sourceToBlock");
        }
        if (networkInterface == null) {
            throw new NullPointerException("networkInterface");
        }
        NioDatagramChannel nioDatagramChannel = this;
        synchronized (nioDatagramChannel) {
            if (this.memberships != null) {
                List<MembershipKey> keys = this.memberships.get(multicastAddress);
                for (MembershipKey key : keys) {
                    if (!networkInterface.equals(key.networkInterface())) continue;
                    try {
                        key.block(sourceToBlock);
                    }
                    catch (IOException e2) {
                        promise.setFailure(e2);
                    }
                }
            }
        }
        promise.setSuccess();
        return promise;
    }

    @Override
    public ChannelFuture block(InetAddress multicastAddress, InetAddress sourceToBlock) {
        return this.block(multicastAddress, sourceToBlock, this.newPromise());
    }

    @Override
    public ChannelFuture block(InetAddress multicastAddress, InetAddress sourceToBlock, ChannelPromise promise) {
        try {
            return this.block(multicastAddress, NetworkInterface.getByInetAddress(this.localAddress().getAddress()), sourceToBlock, promise);
        }
        catch (SocketException e2) {
            promise.setFailure(e2);
            return promise;
        }
    }
}

