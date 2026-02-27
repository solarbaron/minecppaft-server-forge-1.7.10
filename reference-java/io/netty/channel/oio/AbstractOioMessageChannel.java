/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.oio;

import io.netty.channel.Channel;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.oio.AbstractOioChannel;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public abstract class AbstractOioMessageChannel
extends AbstractOioChannel {
    private final List<Object> readBuf = new ArrayList<Object>();

    protected AbstractOioMessageChannel(Channel parent) {
        super(parent);
    }

    @Override
    protected void doRead() {
        ChannelPipeline pipeline = this.pipeline();
        boolean closed = false;
        Throwable exception = null;
        try {
            int localReadAmount = this.doReadMessages(this.readBuf);
            if (localReadAmount < 0) {
                closed = true;
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
                closed = true;
            }
            this.pipeline().fireExceptionCaught(exception);
        }
        if (closed && this.isOpen()) {
            this.unsafe().close(this.unsafe().voidPromise());
        }
    }

    protected abstract int doReadMessages(List<Object> var1) throws Exception;
}

