/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.nio;

import io.netty.channel.Channel;
import io.netty.channel.ChannelConfig;
import io.netty.channel.ChannelOutboundBuffer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.ServerChannel;
import io.netty.channel.nio.AbstractNioChannel;
import java.io.IOException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.util.ArrayList;
import java.util.List;

public abstract class AbstractNioMessageChannel
extends AbstractNioChannel {
    protected AbstractNioMessageChannel(Channel parent, SelectableChannel ch2, int readInterestOp) {
        super(parent, ch2, readInterestOp);
    }

    @Override
    protected AbstractNioChannel.AbstractNioUnsafe newUnsafe() {
        return new NioMessageUnsafe();
    }

    @Override
    protected void doWrite(ChannelOutboundBuffer in2) throws Exception {
        block3: {
            SelectionKey key = this.selectionKey();
            int interestOps = key.interestOps();
            while (true) {
                Object msg;
                if ((msg = in2.current()) == null) {
                    if ((interestOps & 4) == 0) break block3;
                    key.interestOps(interestOps & 0xFFFFFFFB);
                    break block3;
                }
                boolean done = false;
                for (int i2 = this.config().getWriteSpinCount() - 1; i2 >= 0; --i2) {
                    if (!this.doWriteMessage(msg, in2)) continue;
                    done = true;
                    break;
                }
                if (!done) break;
                in2.remove();
            }
            if ((interestOps & 4) != 0) break block3;
            key.interestOps(interestOps | 4);
        }
    }

    protected abstract int doReadMessages(List<Object> var1) throws Exception;

    protected abstract boolean doWriteMessage(Object var1, ChannelOutboundBuffer var2) throws Exception;

    private final class NioMessageUnsafe
    extends AbstractNioChannel.AbstractNioUnsafe {
        private final List<Object> readBuf = new ArrayList<Object>();

        private NioMessageUnsafe() {
        }

        @Override
        public void read() {
            int interestOps;
            assert (AbstractNioMessageChannel.this.eventLoop().inEventLoop());
            SelectionKey key = AbstractNioMessageChannel.this.selectionKey();
            if (!AbstractNioMessageChannel.this.config().isAutoRead() && ((interestOps = key.interestOps()) & AbstractNioMessageChannel.this.readInterestOp) != 0) {
                key.interestOps(interestOps & ~AbstractNioMessageChannel.this.readInterestOp);
            }
            ChannelConfig config = AbstractNioMessageChannel.this.config();
            int maxMessagesPerRead = config.getMaxMessagesPerRead();
            boolean autoRead = config.isAutoRead();
            ChannelPipeline pipeline = AbstractNioMessageChannel.this.pipeline();
            boolean closed = false;
            Throwable exception = null;
            try {
                int localRead;
                while ((localRead = AbstractNioMessageChannel.this.doReadMessages(this.readBuf)) != 0) {
                    if (localRead < 0) {
                        closed = true;
                    } else if (!(this.readBuf.size() >= maxMessagesPerRead | !autoRead)) continue;
                    break;
                }
            }
            catch (Throwable t2) {
                exception = t2;
            }
            int size = this.readBuf.size();
            for (int i2 = 0; i2 < size; ++i2) {
                pipeline.fireChannelRead(this.readBuf.get(i2));
            }
            this.readBuf.clear();
            pipeline.fireChannelReadComplete();
            if (exception != null) {
                if (exception instanceof IOException) {
                    closed = !(AbstractNioMessageChannel.this instanceof ServerChannel);
                }
                pipeline.fireExceptionCaught(exception);
            }
            if (closed && AbstractNioMessageChannel.this.isOpen()) {
                this.close(this.voidPromise());
            }
        }
    }
}

