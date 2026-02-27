/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.socket.nio;

import io.netty.buffer.ByteBuf;
import io.netty.channel.Channel;
import io.netty.channel.ChannelException;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelMetadata;
import io.netty.channel.ChannelOutboundBuffer;
import io.netty.channel.ChannelPromise;
import io.netty.channel.FileRegion;
import io.netty.channel.nio.AbstractNioByteChannel;
import io.netty.channel.nio.NioEventLoop;
import io.netty.channel.socket.DefaultSocketChannelConfig;
import io.netty.channel.socket.ServerSocketChannel;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.SocketChannelConfig;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;

public class NioSocketChannel
extends AbstractNioByteChannel
implements SocketChannel {
    private static final ChannelMetadata METADATA = new ChannelMetadata(false);
    private final SocketChannelConfig config;

    private static java.nio.channels.SocketChannel newSocket() {
        try {
            return java.nio.channels.SocketChannel.open();
        }
        catch (IOException e2) {
            throw new ChannelException("Failed to open a socket.", e2);
        }
    }

    public NioSocketChannel() {
        this(NioSocketChannel.newSocket());
    }

    public NioSocketChannel(java.nio.channels.SocketChannel socket) {
        this(null, socket);
    }

    public NioSocketChannel(Channel parent, java.nio.channels.SocketChannel socket) {
        super(parent, socket);
        this.config = new DefaultSocketChannelConfig(this, socket.socket());
    }

    @Override
    public ServerSocketChannel parent() {
        return (ServerSocketChannel)super.parent();
    }

    @Override
    public ChannelMetadata metadata() {
        return METADATA;
    }

    @Override
    public SocketChannelConfig config() {
        return this.config;
    }

    @Override
    protected java.nio.channels.SocketChannel javaChannel() {
        return (java.nio.channels.SocketChannel)super.javaChannel();
    }

    @Override
    public boolean isActive() {
        java.nio.channels.SocketChannel ch2 = this.javaChannel();
        return ch2.isOpen() && ch2.isConnected();
    }

    @Override
    public boolean isInputShutdown() {
        return super.isInputShutdown();
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
    public boolean isOutputShutdown() {
        return this.javaChannel().socket().isOutputShutdown() || !this.isActive();
    }

    @Override
    public ChannelFuture shutdownOutput() {
        return this.shutdownOutput(this.newPromise());
    }

    @Override
    public ChannelFuture shutdownOutput(final ChannelPromise promise) {
        NioEventLoop loop = this.eventLoop();
        if (loop.inEventLoop()) {
            try {
                this.javaChannel().socket().shutdownOutput();
                promise.setSuccess();
            }
            catch (Throwable t2) {
                promise.setFailure(t2);
            }
        } else {
            loop.execute(new Runnable(){

                @Override
                public void run() {
                    NioSocketChannel.this.shutdownOutput(promise);
                }
            });
        }
        return promise;
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
            boolean connected = this.javaChannel().connect(remoteAddress);
            if (!connected) {
                this.selectionKey().interestOps(8);
            }
            success = true;
            boolean bl2 = connected;
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
        if (!this.javaChannel().finishConnect()) {
            throw new Error();
        }
    }

    @Override
    protected void doDisconnect() throws Exception {
        this.doClose();
    }

    @Override
    protected void doClose() throws Exception {
        this.javaChannel().close();
    }

    @Override
    protected int doReadBytes(ByteBuf byteBuf) throws Exception {
        return byteBuf.writeBytes(this.javaChannel(), byteBuf.writableBytes());
    }

    @Override
    protected int doWriteBytes(ByteBuf buf) throws Exception {
        int expectedWrittenBytes = buf.readableBytes();
        int writtenBytes = buf.readBytes(this.javaChannel(), expectedWrittenBytes);
        return writtenBytes;
    }

    @Override
    protected long doWriteFileRegion(FileRegion region) throws Exception {
        long position = region.transfered();
        long writtenBytes = region.transferTo(this.javaChannel(), position);
        return writtenBytes;
    }

    @Override
    protected void doWrite(ChannelOutboundBuffer in2) throws Exception {
        block9: {
            int i2;
            long writtenBytes;
            block8: {
                do {
                    long localWrittenBytes;
                    int msgCount;
                    if ((msgCount = in2.size()) <= 1) {
                        super.doWrite(in2);
                        return;
                    }
                    ByteBuffer[] nioBuffers = in2.nioBuffers();
                    if (nioBuffers == null) {
                        super.doWrite(in2);
                        return;
                    }
                    int nioBufferCnt = in2.nioBufferCount();
                    long expectedWrittenBytes = in2.nioBufferSize();
                    java.nio.channels.SocketChannel ch2 = this.javaChannel();
                    writtenBytes = 0L;
                    boolean done = false;
                    for (i2 = this.config().getWriteSpinCount() - 1; i2 >= 0 && (localWrittenBytes = ch2.write(nioBuffers, 0, nioBufferCnt)) != 0L; --i2) {
                        writtenBytes += localWrittenBytes;
                        if ((expectedWrittenBytes -= localWrittenBytes) != 0L) continue;
                        done = true;
                        break;
                    }
                    if (!done) break block8;
                    for (i2 = msgCount; i2 > 0; --i2) {
                        in2.remove();
                    }
                } while (!in2.isEmpty());
                this.clearOpWrite();
                break block9;
            }
            for (i2 = msgCount; i2 > 0; --i2) {
                ByteBuf buf = (ByteBuf)in2.current();
                int readerIndex = buf.readerIndex();
                int readableBytes = buf.writerIndex() - readerIndex;
                if ((long)readableBytes < writtenBytes) {
                    in2.progress(readableBytes);
                    in2.remove();
                    writtenBytes -= (long)readableBytes;
                    continue;
                }
                if ((long)readableBytes > writtenBytes) {
                    buf.readerIndex(readerIndex + (int)writtenBytes);
                    in2.progress(writtenBytes);
                    break;
                }
                in2.progress(readableBytes);
                in2.remove();
                break;
            }
            this.setOpWrite();
        }
    }
}

