/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;
import io.netty.handler.codec.spdy.DefaultSpdyDataFrame;
import io.netty.handler.codec.spdy.DefaultSpdyGoAwayFrame;
import io.netty.handler.codec.spdy.DefaultSpdyRstStreamFrame;
import io.netty.handler.codec.spdy.DefaultSpdyWindowUpdateFrame;
import io.netty.handler.codec.spdy.SpdyCodecUtil;
import io.netty.handler.codec.spdy.SpdyDataFrame;
import io.netty.handler.codec.spdy.SpdyGoAwayFrame;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;
import io.netty.handler.codec.spdy.SpdyPingFrame;
import io.netty.handler.codec.spdy.SpdyProtocolException;
import io.netty.handler.codec.spdy.SpdyRstStreamFrame;
import io.netty.handler.codec.spdy.SpdySession;
import io.netty.handler.codec.spdy.SpdySessionStatus;
import io.netty.handler.codec.spdy.SpdySettingsFrame;
import io.netty.handler.codec.spdy.SpdyStreamStatus;
import io.netty.handler.codec.spdy.SpdySynReplyFrame;
import io.netty.handler.codec.spdy.SpdySynStreamFrame;
import io.netty.handler.codec.spdy.SpdyWindowUpdateFrame;
import io.netty.util.internal.EmptyArrays;
import java.util.concurrent.atomic.AtomicInteger;

public class SpdySessionHandler
extends ChannelDuplexHandler {
    private static final SpdyProtocolException PROTOCOL_EXCEPTION = new SpdyProtocolException();
    private static final SpdyProtocolException STREAM_CLOSED = new SpdyProtocolException("Stream closed");
    private final SpdySession spdySession = new SpdySession();
    private int lastGoodStreamId;
    private static final int DEFAULT_MAX_CONCURRENT_STREAMS = Integer.MAX_VALUE;
    private int remoteConcurrentStreams = Integer.MAX_VALUE;
    private int localConcurrentStreams = Integer.MAX_VALUE;
    private int maxConcurrentStreams = Integer.MAX_VALUE;
    private static final int DEFAULT_WINDOW_SIZE = 65536;
    private int initialSendWindowSize = 65536;
    private int initialReceiveWindowSize = 65536;
    private final Object flowControlLock = new Object();
    private final AtomicInteger pings = new AtomicInteger();
    private boolean sentGoAwayFrame;
    private boolean receivedGoAwayFrame;
    private ChannelFutureListener closeSessionFutureListener;
    private final boolean server;
    private final boolean flowControl;

    public SpdySessionHandler(int version, boolean server) {
        if (version < 2 || version > 3) {
            throw new IllegalArgumentException("unsupported version: " + version);
        }
        this.server = server;
        this.flowControl = version >= 3;
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        if (msg instanceof SpdyDataFrame) {
            SpdyDataFrame spdyDataFrame = (SpdyDataFrame)msg;
            int streamId = spdyDataFrame.getStreamId();
            if (!this.spdySession.isActiveStream(streamId)) {
                spdyDataFrame.release();
                if (streamId <= this.lastGoodStreamId) {
                    this.issueStreamError(ctx, streamId, SpdyStreamStatus.PROTOCOL_ERROR);
                } else if (!this.sentGoAwayFrame) {
                    this.issueStreamError(ctx, streamId, SpdyStreamStatus.INVALID_STREAM);
                }
                return;
            }
            if (this.spdySession.isRemoteSideClosed(streamId)) {
                spdyDataFrame.release();
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.STREAM_ALREADY_CLOSED);
                return;
            }
            if (!this.isRemoteInitiatedID(streamId) && !this.spdySession.hasReceivedReply(streamId)) {
                spdyDataFrame.release();
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.PROTOCOL_ERROR);
                return;
            }
            if (this.flowControl) {
                int deltaWindowSize = -1 * spdyDataFrame.content().readableBytes();
                int newWindowSize = this.spdySession.updateReceiveWindowSize(streamId, deltaWindowSize);
                if (newWindowSize < this.spdySession.getReceiveWindowSizeLowerBound(streamId)) {
                    spdyDataFrame.release();
                    this.issueStreamError(ctx, streamId, SpdyStreamStatus.FLOW_CONTROL_ERROR);
                    return;
                }
                if (newWindowSize < 0) {
                    while (spdyDataFrame.content().readableBytes() > this.initialReceiveWindowSize) {
                        DefaultSpdyDataFrame partialDataFrame = new DefaultSpdyDataFrame(streamId, spdyDataFrame.content().readSlice(this.initialReceiveWindowSize).retain());
                        ctx.writeAndFlush(partialDataFrame);
                    }
                }
                if (newWindowSize <= this.initialReceiveWindowSize / 2 && !spdyDataFrame.isLast()) {
                    deltaWindowSize = this.initialReceiveWindowSize - newWindowSize;
                    this.spdySession.updateReceiveWindowSize(streamId, deltaWindowSize);
                    DefaultSpdyWindowUpdateFrame spdyWindowUpdateFrame = new DefaultSpdyWindowUpdateFrame(streamId, deltaWindowSize);
                    ctx.writeAndFlush(spdyWindowUpdateFrame);
                }
            }
            if (spdyDataFrame.isLast()) {
                this.halfCloseStream(streamId, true, ctx.newSucceededFuture());
            }
        } else if (msg instanceof SpdySynStreamFrame) {
            boolean localSideClosed;
            boolean remoteSideClosed;
            SpdySynStreamFrame spdySynStreamFrame = (SpdySynStreamFrame)msg;
            int streamId = spdySynStreamFrame.getStreamId();
            if (spdySynStreamFrame.isInvalid() || !this.isRemoteInitiatedID(streamId) || this.spdySession.isActiveStream(streamId)) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.PROTOCOL_ERROR);
                return;
            }
            if (streamId <= this.lastGoodStreamId) {
                this.issueSessionError(ctx, SpdySessionStatus.PROTOCOL_ERROR);
                return;
            }
            byte priority = spdySynStreamFrame.getPriority();
            if (!this.acceptStream(streamId, priority, remoteSideClosed = spdySynStreamFrame.isLast(), localSideClosed = spdySynStreamFrame.isUnidirectional())) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.REFUSED_STREAM);
                return;
            }
        } else if (msg instanceof SpdySynReplyFrame) {
            SpdySynReplyFrame spdySynReplyFrame = (SpdySynReplyFrame)msg;
            int streamId = spdySynReplyFrame.getStreamId();
            if (spdySynReplyFrame.isInvalid() || this.isRemoteInitiatedID(streamId) || this.spdySession.isRemoteSideClosed(streamId)) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.INVALID_STREAM);
                return;
            }
            if (this.spdySession.hasReceivedReply(streamId)) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.STREAM_IN_USE);
                return;
            }
            this.spdySession.receivedReply(streamId);
            if (spdySynReplyFrame.isLast()) {
                this.halfCloseStream(streamId, true, ctx.newSucceededFuture());
            }
        } else if (msg instanceof SpdyRstStreamFrame) {
            SpdyRstStreamFrame spdyRstStreamFrame = (SpdyRstStreamFrame)msg;
            this.removeStream(spdyRstStreamFrame.getStreamId(), ctx.newSucceededFuture());
        } else if (msg instanceof SpdySettingsFrame) {
            int newInitialWindowSize;
            SpdySettingsFrame spdySettingsFrame = (SpdySettingsFrame)msg;
            int newConcurrentStreams = spdySettingsFrame.getValue(4);
            if (newConcurrentStreams >= 0) {
                this.updateConcurrentStreams(newConcurrentStreams, true);
            }
            if (spdySettingsFrame.isPersisted(7)) {
                spdySettingsFrame.removeValue(7);
            }
            spdySettingsFrame.setPersistValue(7, false);
            if (this.flowControl && (newInitialWindowSize = spdySettingsFrame.getValue(7)) >= 0) {
                this.updateInitialSendWindowSize(newInitialWindowSize);
            }
        } else if (msg instanceof SpdyPingFrame) {
            SpdyPingFrame spdyPingFrame = (SpdyPingFrame)msg;
            if (this.isRemoteInitiatedID(spdyPingFrame.getId())) {
                ctx.writeAndFlush(spdyPingFrame);
                return;
            }
            if (this.pings.get() == 0) {
                return;
            }
            this.pings.getAndDecrement();
        } else if (msg instanceof SpdyGoAwayFrame) {
            this.receivedGoAwayFrame = true;
        } else if (msg instanceof SpdyHeadersFrame) {
            SpdyHeadersFrame spdyHeadersFrame = (SpdyHeadersFrame)msg;
            int streamId = spdyHeadersFrame.getStreamId();
            if (spdyHeadersFrame.isInvalid()) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.PROTOCOL_ERROR);
                return;
            }
            if (this.spdySession.isRemoteSideClosed(streamId)) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.INVALID_STREAM);
                return;
            }
            if (spdyHeadersFrame.isLast()) {
                this.halfCloseStream(streamId, true, ctx.newSucceededFuture());
            }
        } else if (msg instanceof SpdyWindowUpdateFrame && this.flowControl) {
            SpdyWindowUpdateFrame spdyWindowUpdateFrame = (SpdyWindowUpdateFrame)msg;
            int streamId = spdyWindowUpdateFrame.getStreamId();
            int deltaWindowSize = spdyWindowUpdateFrame.getDeltaWindowSize();
            if (this.spdySession.isLocalSideClosed(streamId)) {
                return;
            }
            if (this.spdySession.getSendWindowSize(streamId) > Integer.MAX_VALUE - deltaWindowSize) {
                this.issueStreamError(ctx, streamId, SpdyStreamStatus.FLOW_CONTROL_ERROR);
                return;
            }
            this.updateSendWindowSize(ctx, streamId, deltaWindowSize);
        }
        ctx.fireChannelRead(msg);
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        for (Integer streamId : this.spdySession.getActiveStreams()) {
            this.removeStream(streamId, ctx.newSucceededFuture());
        }
        ctx.fireChannelInactive();
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        if (cause instanceof SpdyProtocolException) {
            this.issueSessionError(ctx, SpdySessionStatus.PROTOCOL_ERROR);
        }
        ctx.fireExceptionCaught(cause);
    }

    @Override
    public void close(ChannelHandlerContext ctx, ChannelPromise promise) throws Exception {
        this.sendGoAwayFrame(ctx, promise);
    }

    @Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {
        if (msg instanceof SpdyDataFrame || msg instanceof SpdySynStreamFrame || msg instanceof SpdySynReplyFrame || msg instanceof SpdyRstStreamFrame || msg instanceof SpdySettingsFrame || msg instanceof SpdyPingFrame || msg instanceof SpdyGoAwayFrame || msg instanceof SpdyHeadersFrame || msg instanceof SpdyWindowUpdateFrame) {
            this.handleOutboundMessage(ctx, msg, promise);
        } else {
            ctx.write(msg, promise);
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void handleOutboundMessage(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {
        if (msg instanceof SpdyDataFrame) {
            SpdyDataFrame spdyDataFrame = (SpdyDataFrame)msg;
            final int streamId = spdyDataFrame.getStreamId();
            if (this.spdySession.isLocalSideClosed(streamId)) {
                spdyDataFrame.release();
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
            if (this.flowControl) {
                Object object = this.flowControlLock;
                synchronized (object) {
                    int dataLength = spdyDataFrame.content().readableBytes();
                    int sendWindowSize = this.spdySession.getSendWindowSize(streamId);
                    if (sendWindowSize <= 0) {
                        this.spdySession.putPendingWrite(streamId, new SpdySession.PendingWrite(spdyDataFrame, promise));
                        return;
                    }
                    if (sendWindowSize < dataLength) {
                        this.spdySession.updateSendWindowSize(streamId, -1 * sendWindowSize);
                        DefaultSpdyDataFrame partialDataFrame = new DefaultSpdyDataFrame(streamId, spdyDataFrame.content().readSlice(sendWindowSize).retain());
                        this.spdySession.putPendingWrite(streamId, new SpdySession.PendingWrite(spdyDataFrame, promise));
                        final ChannelHandlerContext context = ctx;
                        ctx.write(partialDataFrame).addListener(new ChannelFutureListener(){

                            @Override
                            public void operationComplete(ChannelFuture future) throws Exception {
                                if (!future.isSuccess()) {
                                    SpdySessionHandler.this.issueStreamError(context, streamId, SpdyStreamStatus.INTERNAL_ERROR);
                                }
                            }
                        });
                        return;
                    }
                    this.spdySession.updateSendWindowSize(streamId, -1 * dataLength);
                    final ChannelHandlerContext context = ctx;
                    promise.addListener(new ChannelFutureListener(){

                        @Override
                        public void operationComplete(ChannelFuture future) throws Exception {
                            if (!future.isSuccess()) {
                                SpdySessionHandler.this.issueStreamError(context, streamId, SpdyStreamStatus.INTERNAL_ERROR);
                            }
                        }
                    });
                }
            }
            if (spdyDataFrame.isLast()) {
                this.halfCloseStream(streamId, false, promise);
            }
        } else if (msg instanceof SpdySynStreamFrame) {
            boolean localSideClosed;
            boolean remoteSideClosed;
            SpdySynStreamFrame spdySynStreamFrame = (SpdySynStreamFrame)msg;
            int streamId = spdySynStreamFrame.getStreamId();
            if (this.isRemoteInitiatedID(streamId)) {
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
            byte priority = spdySynStreamFrame.getPriority();
            if (!this.acceptStream(streamId, priority, remoteSideClosed = spdySynStreamFrame.isUnidirectional(), localSideClosed = spdySynStreamFrame.isLast())) {
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
        } else if (msg instanceof SpdySynReplyFrame) {
            SpdySynReplyFrame spdySynReplyFrame = (SpdySynReplyFrame)msg;
            int streamId = spdySynReplyFrame.getStreamId();
            if (!this.isRemoteInitiatedID(streamId) || this.spdySession.isLocalSideClosed(streamId)) {
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
            if (spdySynReplyFrame.isLast()) {
                this.halfCloseStream(streamId, false, promise);
            }
        } else if (msg instanceof SpdyRstStreamFrame) {
            SpdyRstStreamFrame spdyRstStreamFrame = (SpdyRstStreamFrame)msg;
            this.removeStream(spdyRstStreamFrame.getStreamId(), promise);
        } else if (msg instanceof SpdySettingsFrame) {
            int newInitialWindowSize;
            SpdySettingsFrame spdySettingsFrame = (SpdySettingsFrame)msg;
            int newConcurrentStreams = spdySettingsFrame.getValue(4);
            if (newConcurrentStreams >= 0) {
                this.updateConcurrentStreams(newConcurrentStreams, false);
            }
            if (spdySettingsFrame.isPersisted(7)) {
                spdySettingsFrame.removeValue(7);
            }
            spdySettingsFrame.setPersistValue(7, false);
            if (this.flowControl && (newInitialWindowSize = spdySettingsFrame.getValue(7)) >= 0) {
                this.updateInitialReceiveWindowSize(newInitialWindowSize);
            }
        } else if (msg instanceof SpdyPingFrame) {
            SpdyPingFrame spdyPingFrame = (SpdyPingFrame)msg;
            if (this.isRemoteInitiatedID(spdyPingFrame.getId())) {
                ctx.fireExceptionCaught(new IllegalArgumentException("invalid PING ID: " + spdyPingFrame.getId()));
                return;
            }
            this.pings.getAndIncrement();
        } else {
            if (msg instanceof SpdyGoAwayFrame) {
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
            if (msg instanceof SpdyHeadersFrame) {
                SpdyHeadersFrame spdyHeadersFrame = (SpdyHeadersFrame)msg;
                int streamId = spdyHeadersFrame.getStreamId();
                if (this.spdySession.isLocalSideClosed(streamId)) {
                    promise.setFailure(PROTOCOL_EXCEPTION);
                    return;
                }
                if (spdyHeadersFrame.isLast()) {
                    this.halfCloseStream(streamId, false, promise);
                }
            } else if (msg instanceof SpdyWindowUpdateFrame) {
                promise.setFailure(PROTOCOL_EXCEPTION);
                return;
            }
        }
        ctx.write(msg, promise);
    }

    private void issueSessionError(ChannelHandlerContext ctx, SpdySessionStatus status) {
        this.sendGoAwayFrame(ctx, status).addListener(new ClosingChannelFutureListener(ctx, ctx.newPromise()));
    }

    private void issueStreamError(ChannelHandlerContext ctx, int streamId, SpdyStreamStatus status) {
        boolean fireChannelRead = !this.spdySession.isRemoteSideClosed(streamId);
        ChannelPromise promise = ctx.newPromise();
        this.removeStream(streamId, promise);
        DefaultSpdyRstStreamFrame spdyRstStreamFrame = new DefaultSpdyRstStreamFrame(streamId, status);
        ctx.writeAndFlush(spdyRstStreamFrame, promise);
        if (fireChannelRead) {
            ctx.fireChannelRead(spdyRstStreamFrame);
        }
    }

    private boolean isRemoteInitiatedID(int id2) {
        boolean serverId = SpdyCodecUtil.isServerId(id2);
        return this.server && !serverId || !this.server && serverId;
    }

    private void updateConcurrentStreams(int newConcurrentStreams, boolean remote) {
        if (remote) {
            this.remoteConcurrentStreams = newConcurrentStreams;
        } else {
            this.localConcurrentStreams = newConcurrentStreams;
        }
        this.maxConcurrentStreams = Math.min(this.localConcurrentStreams, this.remoteConcurrentStreams);
    }

    private synchronized void updateInitialSendWindowSize(int newInitialWindowSize) {
        int deltaWindowSize = newInitialWindowSize - this.initialSendWindowSize;
        this.initialSendWindowSize = newInitialWindowSize;
        this.spdySession.updateAllSendWindowSizes(deltaWindowSize);
    }

    private synchronized void updateInitialReceiveWindowSize(int newInitialWindowSize) {
        int deltaWindowSize = newInitialWindowSize - this.initialReceiveWindowSize;
        this.initialReceiveWindowSize = newInitialWindowSize;
        this.spdySession.updateAllReceiveWindowSizes(deltaWindowSize);
    }

    private synchronized boolean acceptStream(int streamId, byte priority, boolean remoteSideClosed, boolean localSideClosed) {
        if (this.receivedGoAwayFrame || this.sentGoAwayFrame) {
            return false;
        }
        if (this.spdySession.numActiveStreams() >= this.maxConcurrentStreams) {
            return false;
        }
        this.spdySession.acceptStream(streamId, priority, remoteSideClosed, localSideClosed, this.initialSendWindowSize, this.initialReceiveWindowSize);
        if (this.isRemoteInitiatedID(streamId)) {
            this.lastGoodStreamId = streamId;
        }
        return true;
    }

    private void halfCloseStream(int streamId, boolean remote, ChannelFuture future) {
        if (remote) {
            this.spdySession.closeRemoteSide(streamId);
        } else {
            this.spdySession.closeLocalSide(streamId);
        }
        if (this.closeSessionFutureListener != null && this.spdySession.noActiveStreams()) {
            future.addListener(this.closeSessionFutureListener);
        }
    }

    private void removeStream(int streamId, ChannelFuture future) {
        this.spdySession.removeStream(streamId, STREAM_CLOSED);
        if (this.closeSessionFutureListener != null && this.spdySession.noActiveStreams()) {
            future.addListener(this.closeSessionFutureListener);
        }
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void updateSendWindowSize(final ChannelHandlerContext ctx, final int streamId, int deltaWindowSize) {
        Object object = this.flowControlLock;
        synchronized (object) {
            SpdySession.PendingWrite pendingWrite;
            int newWindowSize = this.spdySession.updateSendWindowSize(streamId, deltaWindowSize);
            while (newWindowSize > 0 && (pendingWrite = this.spdySession.getPendingWrite(streamId)) != null) {
                SpdyDataFrame spdyDataFrame = pendingWrite.spdyDataFrame;
                int dataFrameSize = spdyDataFrame.content().readableBytes();
                if (newWindowSize >= dataFrameSize) {
                    this.spdySession.removePendingWrite(streamId);
                    newWindowSize = this.spdySession.updateSendWindowSize(streamId, -1 * dataFrameSize);
                    if (spdyDataFrame.isLast()) {
                        this.halfCloseStream(streamId, false, pendingWrite.promise);
                    }
                    ctx.writeAndFlush(spdyDataFrame, pendingWrite.promise).addListener(new ChannelFutureListener(){

                        @Override
                        public void operationComplete(ChannelFuture future) throws Exception {
                            if (!future.isSuccess()) {
                                SpdySessionHandler.this.issueStreamError(ctx, streamId, SpdyStreamStatus.INTERNAL_ERROR);
                            }
                        }
                    });
                    continue;
                }
                this.spdySession.updateSendWindowSize(streamId, -1 * newWindowSize);
                DefaultSpdyDataFrame partialDataFrame = new DefaultSpdyDataFrame(streamId, spdyDataFrame.content().readSlice(newWindowSize).retain());
                ctx.writeAndFlush(partialDataFrame).addListener(new ChannelFutureListener(){

                    @Override
                    public void operationComplete(ChannelFuture future) throws Exception {
                        if (!future.isSuccess()) {
                            SpdySessionHandler.this.issueStreamError(ctx, streamId, SpdyStreamStatus.INTERNAL_ERROR);
                        }
                    }
                });
                newWindowSize = 0;
            }
        }
    }

    private void sendGoAwayFrame(ChannelHandlerContext ctx, ChannelPromise future) {
        if (!ctx.channel().isActive()) {
            ctx.close(future);
            return;
        }
        ChannelFuture f2 = this.sendGoAwayFrame(ctx, SpdySessionStatus.OK);
        if (this.spdySession.noActiveStreams()) {
            f2.addListener(new ClosingChannelFutureListener(ctx, future));
        } else {
            this.closeSessionFutureListener = new ClosingChannelFutureListener(ctx, future);
        }
    }

    private synchronized ChannelFuture sendGoAwayFrame(ChannelHandlerContext ctx, SpdySessionStatus status) {
        if (!this.sentGoAwayFrame) {
            this.sentGoAwayFrame = true;
            DefaultSpdyGoAwayFrame spdyGoAwayFrame = new DefaultSpdyGoAwayFrame(this.lastGoodStreamId, status);
            return ctx.writeAndFlush(spdyGoAwayFrame);
        }
        return ctx.newSucceededFuture();
    }

    static {
        PROTOCOL_EXCEPTION.setStackTrace(EmptyArrays.EMPTY_STACK_TRACE);
        STREAM_CLOSED.setStackTrace(EmptyArrays.EMPTY_STACK_TRACE);
    }

    private static final class ClosingChannelFutureListener
    implements ChannelFutureListener {
        private final ChannelHandlerContext ctx;
        private final ChannelPromise promise;

        ClosingChannelFutureListener(ChannelHandlerContext ctx, ChannelPromise promise) {
            this.ctx = ctx;
            this.promise = promise;
        }

        @Override
        public void operationComplete(ChannelFuture sentGoAwayFuture) throws Exception {
            this.ctx.close(this.promise);
        }
    }
}

