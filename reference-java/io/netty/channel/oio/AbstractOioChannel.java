/*
 * Decompiled with CFR 0.152.
 */
package io.netty.channel.oio;

import io.netty.channel.AbstractChannel;
import io.netty.channel.Channel;
import io.netty.channel.ChannelPromise;
import io.netty.channel.EventLoop;
import io.netty.channel.ThreadPerChannelEventLoop;
import java.net.ConnectException;
import java.net.SocketAddress;

public abstract class AbstractOioChannel
extends AbstractChannel {
    protected static final int SO_TIMEOUT = 1000;
    private boolean readInProgress;
    private final Runnable readTask = new Runnable(){

        @Override
        public void run() {
            AbstractOioChannel.this.readInProgress = false;
            AbstractOioChannel.this.doRead();
        }
    };

    protected AbstractOioChannel(Channel parent) {
        super(parent);
    }

    @Override
    protected AbstractChannel.AbstractUnsafe newUnsafe() {
        return new DefaultOioUnsafe();
    }

    @Override
    protected boolean isCompatible(EventLoop loop) {
        return loop instanceof ThreadPerChannelEventLoop;
    }

    protected abstract void doConnect(SocketAddress var1, SocketAddress var2) throws Exception;

    @Override
    protected void doBeginRead() throws Exception {
        if (this.readInProgress) {
            return;
        }
        this.readInProgress = true;
        this.eventLoop().execute(this.readTask);
    }

    protected abstract void doRead();

    private final class DefaultOioUnsafe
    extends AbstractChannel.AbstractUnsafe {
        private DefaultOioUnsafe() {
        }

        @Override
        public void connect(SocketAddress remoteAddress, SocketAddress localAddress, ChannelPromise promise) {
            if (!this.ensureOpen(promise)) {
                return;
            }
            if (!promise.setUncancellable()) {
                this.close(this.voidPromise());
                return;
            }
            try {
                boolean wasActive = AbstractOioChannel.this.isActive();
                AbstractOioChannel.this.doConnect(remoteAddress, localAddress);
                promise.setSuccess();
                if (!wasActive && AbstractOioChannel.this.isActive()) {
                    AbstractOioChannel.this.pipeline().fireChannelActive();
                }
            }
            catch (Throwable t22) {
                ConnectException t22;
                if (t22 instanceof ConnectException) {
                    ConnectException newT = new ConnectException(t22.getMessage() + ": " + remoteAddress);
                    newT.setStackTrace(t22.getStackTrace());
                    t22 = newT;
                }
                this.closeIfClosed();
                promise.setFailure(t22);
            }
        }
    }
}

