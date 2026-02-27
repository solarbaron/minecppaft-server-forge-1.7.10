/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.nio;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.Channel;
import io.netty.channel.ChannelConfig;
import io.netty.channel.ChannelOption;
import io.netty.channel.ChannelOutboundBuffer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.FileRegion;
import io.netty.channel.RecvByteBufAllocator;
import io.netty.channel.nio.AbstractNioChannel;
import io.netty.channel.socket.ChannelInputShutdownEvent;
import io.netty.util.internal.StringUtil;
import java.io.IOException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;

public abstract class AbstractNioByteChannel
extends AbstractNioChannel {
    protected AbstractNioByteChannel(Channel parent, SelectableChannel ch2) {
        super(parent, ch2, 1);
    }

    @Override
    protected AbstractNioChannel.AbstractNioUnsafe newUnsafe() {
        return new NioByteUnsafe();
    }

    @Override
    protected void doWrite(ChannelOutboundBuffer in2) throws Exception {
        block10: {
            Object msg;
            block11: {
                int writeSpinCount = -1;
                while (true) {
                    long localFlushedAmount;
                    if ((msg = in2.current()) == null) {
                        this.clearOpWrite();
                        break block10;
                    }
                    if (msg instanceof ByteBuf) {
                        int localFlushedAmount2;
                        ByteBufAllocator alloc;
                        ByteBuf buf = (ByteBuf)msg;
                        int readableBytes = buf.readableBytes();
                        if (readableBytes == 0) {
                            in2.remove();
                            continue;
                        }
                        if (!buf.isDirect() && (alloc = this.alloc()).isDirectBufferPooled()) {
                            buf = alloc.directBuffer(readableBytes).writeBytes(buf);
                            in2.current(buf);
                        }
                        boolean done = false;
                        long flushedAmount = 0L;
                        if (writeSpinCount == -1) {
                            writeSpinCount = this.config().getWriteSpinCount();
                        }
                        for (int i2 = writeSpinCount - 1; i2 >= 0 && (localFlushedAmount2 = this.doWriteBytes(buf)) != 0; --i2) {
                            flushedAmount += (long)localFlushedAmount2;
                            if (buf.isReadable()) continue;
                            done = true;
                            break;
                        }
                        in2.progress(flushedAmount);
                        if (done) {
                            in2.remove();
                            continue;
                        }
                        this.setOpWrite();
                        break block10;
                    }
                    if (!(msg instanceof FileRegion)) break block11;
                    FileRegion region = (FileRegion)msg;
                    boolean done = false;
                    long flushedAmount = 0L;
                    if (writeSpinCount == -1) {
                        writeSpinCount = this.config().getWriteSpinCount();
                    }
                    for (int i3 = writeSpinCount - 1; i3 >= 0 && (localFlushedAmount = this.doWriteFileRegion(region)) != 0L; --i3) {
                        flushedAmount += localFlushedAmount;
                        if (region.transfered() < region.count()) continue;
                        done = true;
                        break;
                    }
                    in2.progress(flushedAmount);
                    if (!done) break;
                    in2.remove();
                }
                this.setOpWrite();
                break block10;
            }
            throw new UnsupportedOperationException("unsupported message type: " + StringUtil.simpleClassName(msg));
        }
    }

    protected abstract long doWriteFileRegion(FileRegion var1) throws Exception;

    protected abstract int doReadBytes(ByteBuf var1) throws Exception;

    protected abstract int doWriteBytes(ByteBuf var1) throws Exception;

    protected final void setOpWrite() {
        SelectionKey key = this.selectionKey();
        int interestOps = key.interestOps();
        if ((interestOps & 4) == 0) {
            key.interestOps(interestOps | 4);
        }
    }

    protected final void clearOpWrite() {
        SelectionKey key = this.selectionKey();
        int interestOps = key.interestOps();
        if ((interestOps & 4) != 0) {
            key.interestOps(interestOps & 0xFFFFFFFB);
        }
    }

    private final class NioByteUnsafe
    extends AbstractNioChannel.AbstractNioUnsafe {
        private RecvByteBufAllocator.Handle allocHandle;

        private NioByteUnsafe() {
            super(AbstractNioByteChannel.this);
        }

        /*
         * WARNING - Removed try catching itself - possible behaviour change.
         */
        @Override
        public void read() {
            int interestOps;
            assert (AbstractNioByteChannel.this.eventLoop().inEventLoop());
            SelectionKey key = AbstractNioByteChannel.this.selectionKey();
            ChannelConfig config = AbstractNioByteChannel.this.config();
            if (!config.isAutoRead() && ((interestOps = key.interestOps()) & AbstractNioByteChannel.this.readInterestOp) != 0) {
                key.interestOps(interestOps & ~AbstractNioByteChannel.this.readInterestOp);
            }
            ChannelPipeline pipeline = AbstractNioByteChannel.this.pipeline();
            RecvByteBufAllocator.Handle allocHandle = this.allocHandle;
            if (allocHandle == null) {
                this.allocHandle = allocHandle = config.getRecvByteBufAllocator().newHandle();
            }
            ByteBufAllocator allocator = config.getAllocator();
            int maxMessagesPerRead = config.getMaxMessagesPerRead();
            boolean closed = false;
            Throwable exception = null;
            ByteBuf byteBuf = null;
            int messages = 0;
            try {
                do {
                    int localReadAmount;
                    if ((localReadAmount = AbstractNioByteChannel.this.doReadBytes(byteBuf = allocHandle.allocate(allocator))) == 0) {
                        byteBuf.release();
                        byteBuf = null;
                        break;
                    }
                    if (localReadAmount < 0) {
                        closed = true;
                        byteBuf.release();
                        byteBuf = null;
                        break;
                    }
                    pipeline.fireChannelRead(byteBuf);
                    allocHandle.record(localReadAmount);
                    byteBuf = null;
                } while (++messages != maxMessagesPerRead);
            }
            catch (Throwable t2) {
                exception = t2;
            }
            finally {
                if (byteBuf != null) {
                    if (byteBuf.isReadable()) {
                        pipeline.fireChannelRead(byteBuf);
                    } else {
                        byteBuf.release();
                    }
                }
                pipeline.fireChannelReadComplete();
                if (exception != null) {
                    if (exception instanceof IOException) {
                        closed = true;
                    }
                    AbstractNioByteChannel.this.pipeline().fireExceptionCaught(exception);
                }
                if (closed) {
                    AbstractNioByteChannel.this.setInputShutdown();
                    if (AbstractNioByteChannel.this.isOpen()) {
                        if (Boolean.TRUE.equals(AbstractNioByteChannel.this.config().getOption(ChannelOption.ALLOW_HALF_CLOSURE))) {
                            key.interestOps(key.interestOps() & ~AbstractNioByteChannel.this.readInterestOp);
                            pipeline.fireUserEventTriggered(ChannelInputShutdownEvent.INSTANCE);
                        } else {
                            this.close(this.voidPromise());
                        }
                    }
                }
            }
        }
    }
}

