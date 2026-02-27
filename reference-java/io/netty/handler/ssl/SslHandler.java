/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.ssl;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.buffer.Unpooled;
import io.netty.channel.Channel;
import io.netty.channel.ChannelException;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelOutboundHandler;
import io.netty.channel.ChannelPromise;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.ssl.NotSslRecordException;
import io.netty.handler.ssl.SslHandshakeCompletionEvent;
import io.netty.util.concurrent.DefaultPromise;
import io.netty.util.concurrent.EventExecutor;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;
import io.netty.util.concurrent.ImmediateExecutor;
import io.netty.util.concurrent.ScheduledFuture;
import io.netty.util.internal.EmptyArrays;
import io.netty.util.internal.PendingWrite;
import io.netty.util.internal.PlatformDependent;
import io.netty.util.internal.RecyclableArrayList;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLEngineResult;
import javax.net.ssl.SSLException;

public class SslHandler
extends ByteToMessageDecoder
implements ChannelOutboundHandler {
    private static final InternalLogger logger = InternalLoggerFactory.getInstance(SslHandler.class);
    private static final Pattern IGNORABLE_CLASS_IN_STACK = Pattern.compile("^.*(?:Socket|Datagram|Sctp|Udt)Channel.*$");
    private static final Pattern IGNORABLE_ERROR_MESSAGE = Pattern.compile("^.*(?:connection.*(?:reset|closed|abort|broken)|broken.*pipe).*$", 2);
    private static final SSLException SSLENGINE_CLOSED = new SSLException("SSLEngine closed already");
    private static final SSLException HANDSHAKE_TIMED_OUT = new SSLException("handshake timed out");
    private static final ClosedChannelException CHANNEL_CLOSED = new ClosedChannelException();
    private volatile ChannelHandlerContext ctx;
    private final SSLEngine engine;
    private final Executor delegatedTaskExecutor;
    private final boolean startTls;
    private boolean sentFirstMessage;
    private final LazyChannelPromise handshakePromise = new LazyChannelPromise();
    private final LazyChannelPromise sslCloseFuture = new LazyChannelPromise();
    private final Deque<PendingWrite> pendingUnencryptedWrites = new ArrayDeque<PendingWrite>();
    private int packetLength;
    private ByteBuf decodeOut;
    private volatile long handshakeTimeoutMillis = 10000L;
    private volatile long closeNotifyTimeoutMillis = 3000L;

    public SslHandler(SSLEngine engine) {
        this(engine, ImmediateExecutor.INSTANCE);
    }

    public SslHandler(SSLEngine engine, boolean startTls) {
        this(engine, startTls, ImmediateExecutor.INSTANCE);
    }

    public SslHandler(SSLEngine engine, Executor delegatedTaskExecutor) {
        this(engine, false, delegatedTaskExecutor);
    }

    public SslHandler(SSLEngine engine, boolean startTls, Executor delegatedTaskExecutor) {
        if (engine == null) {
            throw new NullPointerException("engine");
        }
        if (delegatedTaskExecutor == null) {
            throw new NullPointerException("delegatedTaskExecutor");
        }
        this.engine = engine;
        this.delegatedTaskExecutor = delegatedTaskExecutor;
        this.startTls = startTls;
    }

    public long getHandshakeTimeoutMillis() {
        return this.handshakeTimeoutMillis;
    }

    public void setHandshakeTimeout(long handshakeTimeout, TimeUnit unit) {
        if (unit == null) {
            throw new NullPointerException("unit");
        }
        this.setHandshakeTimeoutMillis(unit.toMillis(handshakeTimeout));
    }

    public void setHandshakeTimeoutMillis(long handshakeTimeoutMillis) {
        if (handshakeTimeoutMillis < 0L) {
            throw new IllegalArgumentException("handshakeTimeoutMillis: " + handshakeTimeoutMillis + " (expected: >= 0)");
        }
        this.handshakeTimeoutMillis = handshakeTimeoutMillis;
    }

    public long getCloseNotifyTimeoutMillis() {
        return this.closeNotifyTimeoutMillis;
    }

    public void setCloseNotifyTimeout(long closeNotifyTimeout, TimeUnit unit) {
        if (unit == null) {
            throw new NullPointerException("unit");
        }
        this.setCloseNotifyTimeoutMillis(unit.toMillis(closeNotifyTimeout));
    }

    public void setCloseNotifyTimeoutMillis(long closeNotifyTimeoutMillis) {
        if (closeNotifyTimeoutMillis < 0L) {
            throw new IllegalArgumentException("closeNotifyTimeoutMillis: " + closeNotifyTimeoutMillis + " (expected: >= 0)");
        }
        this.closeNotifyTimeoutMillis = closeNotifyTimeoutMillis;
    }

    public SSLEngine engine() {
        return this.engine;
    }

    public Future<Channel> handshakeFuture() {
        return this.handshakePromise;
    }

    public ChannelFuture close() {
        return this.close(this.ctx.newPromise());
    }

    public ChannelFuture close(final ChannelPromise future) {
        final ChannelHandlerContext ctx = this.ctx;
        ctx.executor().execute(new Runnable(){

            @Override
            public void run() {
                block2: {
                    SslHandler.this.engine.closeOutbound();
                    try {
                        SslHandler.this.write(ctx, Unpooled.EMPTY_BUFFER, future);
                        SslHandler.this.flush(ctx);
                    }
                    catch (Exception e2) {
                        if (future.tryFailure(e2)) break block2;
                        logger.warn("flush() raised a masked exception.", e2);
                    }
                }
            }
        });
        return future;
    }

    public Future<Channel> sslCloseFuture() {
        return this.sslCloseFuture;
    }

    @Override
    public void handlerRemoved0(ChannelHandlerContext ctx) throws Exception {
        PendingWrite write;
        if (this.decodeOut != null) {
            this.decodeOut.release();
            this.decodeOut = null;
        }
        while ((write = this.pendingUnencryptedWrites.poll()) != null) {
            write.failAndRecycle(new ChannelException("Pending write on removal of SslHandler"));
        }
    }

    @Override
    public void bind(ChannelHandlerContext ctx, SocketAddress localAddress, ChannelPromise promise) throws Exception {
        ctx.bind(localAddress, promise);
    }

    @Override
    public void connect(ChannelHandlerContext ctx, SocketAddress remoteAddress, SocketAddress localAddress, ChannelPromise promise) throws Exception {
        ctx.connect(remoteAddress, localAddress, promise);
    }

    @Override
    public void deregister(ChannelHandlerContext ctx, ChannelPromise promise) throws Exception {
        ctx.deregister(promise);
    }

    @Override
    public void disconnect(ChannelHandlerContext ctx, ChannelPromise promise) throws Exception {
        this.closeOutboundAndChannel(ctx, promise, true);
    }

    @Override
    public void close(ChannelHandlerContext ctx, ChannelPromise promise) throws Exception {
        this.closeOutboundAndChannel(ctx, promise, false);
    }

    @Override
    public void read(ChannelHandlerContext ctx) {
        ctx.read();
    }

    @Override
    public void write(ChannelHandlerContext ctx, Object msg, ChannelPromise promise) throws Exception {
        this.pendingUnencryptedWrites.add(PendingWrite.newInstance((ByteBuf)msg, promise));
    }

    @Override
    public void flush(ChannelHandlerContext ctx) throws Exception {
        if (this.startTls && !this.sentFirstMessage) {
            PendingWrite pendingWrite;
            this.sentFirstMessage = true;
            while ((pendingWrite = this.pendingUnencryptedWrites.poll()) != null) {
                ctx.write(pendingWrite.msg(), (ChannelPromise)pendingWrite.recycleAndGet());
            }
            ctx.flush();
            return;
        }
        if (this.pendingUnencryptedWrites.isEmpty()) {
            this.pendingUnencryptedWrites.add(PendingWrite.newInstance(Unpooled.EMPTY_BUFFER, null));
        }
        this.flush0(ctx);
    }

    private void flush0(ChannelHandlerContext ctx) throws SSLException {
        block29: {
            ByteBuf out = null;
            ChannelPromise promise = null;
            try {
                SSLEngineResult result;
                block13: while (true) {
                    PendingWrite pending;
                    if ((pending = this.pendingUnencryptedWrites.peek()) == null) {
                        break block29;
                    }
                    if (out == null) {
                        out = ctx.alloc().buffer();
                    }
                    ByteBuf buf = (ByteBuf)pending.msg();
                    result = SslHandler.wrap(this.engine, buf, out);
                    if (!buf.isReadable()) {
                        buf.release();
                        promise = (ChannelPromise)pending.recycleAndGet();
                        this.pendingUnencryptedWrites.remove();
                    } else {
                        promise = null;
                    }
                    if (result.getStatus() == SSLEngineResult.Status.CLOSED) {
                        PendingWrite w;
                        while ((w = this.pendingUnencryptedWrites.poll()) != null) {
                            w.failAndRecycle(SSLENGINE_CLOSED);
                        }
                        return;
                    }
                    switch (result.getHandshakeStatus()) {
                        case NEED_TASK: {
                            this.runDelegatedTasks();
                            continue block13;
                        }
                        case FINISHED: {
                            this.setHandshakeSuccess();
                        }
                        case NOT_HANDSHAKING: 
                        case NEED_WRAP: {
                            if (promise != null) {
                                ctx.writeAndFlush(out, promise);
                                promise = null;
                            } else {
                                ctx.writeAndFlush(out);
                            }
                            out = ctx.alloc().buffer();
                            continue block13;
                        }
                        case NEED_UNWRAP: {
                            return;
                        }
                    }
                    break;
                }
                throw new IllegalStateException("Unknown handshake status: " + (Object)((Object)result.getHandshakeStatus()));
            }
            catch (SSLException e2) {
                this.setHandshakeFailure(e2);
                throw e2;
            }
            finally {
                if (out != null && out.isReadable()) {
                    if (promise != null) {
                        ctx.writeAndFlush(out, promise);
                    } else {
                        ctx.writeAndFlush(out);
                    }
                    out = null;
                } else if (promise != null) {
                    promise.trySuccess();
                }
                if (out != null) {
                    out.release();
                }
            }
        }
    }

    private void flushNonAppData0(ChannelHandlerContext ctx, boolean inUnwrap) throws SSLException {
        ByteBuf out = null;
        try {
            SSLEngineResult result;
            block12: do {
                if (out == null) {
                    out = ctx.alloc().buffer();
                }
                if ((result = SslHandler.wrap(this.engine, Unpooled.EMPTY_BUFFER, out)).bytesProduced() > 0) {
                    ctx.writeAndFlush(out);
                    out = null;
                }
                switch (result.getHandshakeStatus()) {
                    case FINISHED: {
                        this.setHandshakeSuccess();
                        break;
                    }
                    case NEED_TASK: {
                        this.runDelegatedTasks();
                        break;
                    }
                    case NEED_UNWRAP: {
                        if (inUnwrap) continue block12;
                        this.unwrap(ctx);
                        break;
                    }
                    case NEED_WRAP: {
                        break;
                    }
                    case NOT_HANDSHAKING: {
                        if (inUnwrap) continue block12;
                        this.unwrap(ctx);
                        break;
                    }
                    default: {
                        throw new IllegalStateException("Unknown handshake status: " + (Object)((Object)result.getHandshakeStatus()));
                    }
                }
            } while (result.bytesProduced() != 0);
        }
        catch (SSLException e2) {
            this.setHandshakeFailure(e2);
            throw e2;
        }
        finally {
            if (out != null) {
                out.release();
            }
        }
    }

    private static SSLEngineResult wrap(SSLEngine engine, ByteBuf in2, ByteBuf out) throws SSLException {
        SSLEngineResult result;
        ByteBuffer in0 = in2.nioBuffer();
        while (true) {
            ByteBuffer out0 = out.nioBuffer(out.writerIndex(), out.writableBytes());
            result = engine.wrap(in0, out0);
            in2.skipBytes(result.bytesConsumed());
            out.writerIndex(out.writerIndex() + result.bytesProduced());
            if (result.getStatus() != SSLEngineResult.Status.BUFFER_OVERFLOW) break;
            out.ensureWritable(engine.getSession().getPacketBufferSize());
        }
        return result;
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        this.setHandshakeFailure(CHANNEL_CLOSED);
        super.channelInactive(ctx);
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) throws Exception {
        if (this.ignoreException(cause)) {
            if (logger.isDebugEnabled()) {
                logger.debug("Swallowing a harmless 'connection reset by peer / broken pipe' error that occurred while writing close_notify in response to the peer's close_notify", cause);
            }
            if (ctx.channel().isActive()) {
                ctx.close();
            }
        } else {
            ctx.fireExceptionCaught(cause);
        }
    }

    private boolean ignoreException(Throwable t2) {
        if (!(t2 instanceof SSLException) && t2 instanceof IOException && this.sslCloseFuture.isDone()) {
            StackTraceElement[] elements;
            String message = String.valueOf(t2.getMessage()).toLowerCase();
            if (IGNORABLE_ERROR_MESSAGE.matcher(message).matches()) {
                return true;
            }
            for (StackTraceElement element : elements = t2.getStackTrace()) {
                String classname = element.getClassName();
                String methodname = element.getMethodName();
                if (classname.startsWith("io.netty.") || !"read".equals(methodname)) continue;
                if (IGNORABLE_CLASS_IN_STACK.matcher(classname).matches()) {
                    return true;
                }
                try {
                    Class<?> clazz = this.getClass().getClassLoader().loadClass(classname);
                    if (SocketChannel.class.isAssignableFrom(clazz) || DatagramChannel.class.isAssignableFrom(clazz)) {
                        return true;
                    }
                    if (PlatformDependent.javaVersion() >= 7 && "com.sun.nio.sctp.SctpChannel".equals(clazz.getSuperclass().getName())) {
                        return true;
                    }
                }
                catch (ClassNotFoundException e2) {
                    // empty catch block
                }
            }
        }
        return false;
    }

    public static boolean isEncrypted(ByteBuf buffer) {
        if (buffer.readableBytes() < 5) {
            throw new IllegalArgumentException("buffer must have at least 5 readable bytes");
        }
        return SslHandler.getEncryptedPacketLength(buffer) != -1;
    }

    private static int getEncryptedPacketLength(ByteBuf buffer) {
        boolean tls;
        int first = buffer.readerIndex();
        int packetLength = 0;
        switch (buffer.getUnsignedByte(first)) {
            case 20: 
            case 21: 
            case 22: 
            case 23: {
                tls = true;
                break;
            }
            default: {
                tls = false;
            }
        }
        if (tls) {
            short majorVersion = buffer.getUnsignedByte(first + 1);
            if (majorVersion == 3) {
                packetLength = buffer.getUnsignedShort(first + 3) + 5;
                if (packetLength <= 5) {
                    tls = false;
                }
            } else {
                tls = false;
            }
        }
        if (!tls) {
            boolean sslv2 = true;
            int headerLength = (buffer.getUnsignedByte(first) & 0x80) != 0 ? 2 : 3;
            short majorVersion = buffer.getUnsignedByte(first + headerLength + 1);
            if (majorVersion == 2 || majorVersion == 3) {
                packetLength = headerLength == 2 ? (buffer.getShort(first) & Short.MAX_VALUE) + 2 : (buffer.getShort(first) & 0x3FFF) + 3;
                if (packetLength <= headerLength) {
                    sslv2 = false;
                }
            } else {
                sslv2 = false;
            }
            if (!sslv2) {
                return -1;
            }
        }
        return packetLength;
    }

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in2, List<Object> out) throws SSLException {
        int packetLength = this.packetLength;
        int readableBytes = in2.readableBytes();
        if (packetLength == 0) {
            if (readableBytes < 5) {
                return;
            }
            packetLength = SslHandler.getEncryptedPacketLength(in2);
            if (packetLength == -1) {
                NotSslRecordException e2 = new NotSslRecordException("not an SSL/TLS record: " + ByteBufUtil.hexDump(in2));
                in2.skipBytes(readableBytes);
                ctx.fireExceptionCaught(e2);
                this.setHandshakeFailure(e2);
                return;
            }
            assert (packetLength > 0);
            this.packetLength = packetLength;
        }
        if (readableBytes < packetLength) {
            return;
        }
        int readerIndex = in2.readerIndex();
        in2.skipBytes(packetLength);
        ByteBuffer buffer = in2.nioBuffer(readerIndex, packetLength);
        this.packetLength = 0;
        this.unwrap(ctx, buffer, out);
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    private void unwrap(ChannelHandlerContext ctx) throws SSLException {
        RecyclableArrayList out = RecyclableArrayList.newInstance();
        try {
            this.unwrap(ctx, Unpooled.EMPTY_BUFFER.nioBuffer(), out);
            int size = out.size();
            for (int i2 = 0; i2 < size; ++i2) {
                ctx.fireChannelRead(out.get(i2));
            }
        }
        finally {
            out.recycle();
        }
    }

    /*
     * Enabled aggressive block sorting
     * Enabled unnecessary exception pruning
     * Enabled aggressive exception aggregation
     */
    private void unwrap(ChannelHandlerContext ctx, ByteBuffer packet, List<Object> out) throws SSLException {
        boolean wrapLater = false;
        int bytesProduced = 0;
        try {
            block16: while (true) {
                if (this.decodeOut == null) {
                    this.decodeOut = ctx.alloc().buffer();
                }
                SSLEngineResult result = SslHandler.unwrap(this.engine, packet, this.decodeOut);
                bytesProduced += result.bytesProduced();
                switch (result.getStatus()) {
                    case CLOSED: {
                        this.sslCloseFuture.trySuccess(ctx.channel());
                        break;
                    }
                    case BUFFER_UNDERFLOW: {
                        break block16;
                    }
                }
                switch (result.getHandshakeStatus()) {
                    case NEED_UNWRAP: {
                        break;
                    }
                    case NEED_WRAP: {
                        this.flushNonAppData0(ctx, true);
                        break;
                    }
                    case NEED_TASK: {
                        this.runDelegatedTasks();
                        break;
                    }
                    case FINISHED: {
                        this.setHandshakeSuccess();
                        wrapLater = true;
                        continue block16;
                    }
                    case NOT_HANDSHAKING: {
                        break;
                    }
                    default: {
                        throw new IllegalStateException("Unknown handshake status: " + (Object)((Object)result.getHandshakeStatus()));
                    }
                }
                if (result.bytesConsumed() == 0 && result.bytesProduced() == 0) break;
            }
            if (!wrapLater) return;
            this.flush0(ctx);
            return;
        }
        catch (SSLException e2) {
            this.setHandshakeFailure(e2);
            throw e2;
        }
        finally {
            if (bytesProduced > 0) {
                ByteBuf decodeOut = this.decodeOut;
                this.decodeOut = null;
                out.add(decodeOut);
            }
        }
    }

    private static SSLEngineResult unwrap(SSLEngine engine, ByteBuffer in2, ByteBuf out) throws SSLException {
        SSLEngineResult result;
        block3: while (true) {
            ByteBuffer out0 = out.nioBuffer(out.writerIndex(), out.writableBytes());
            result = engine.unwrap(in2, out0);
            out.writerIndex(out.writerIndex() + result.bytesProduced());
            switch (result.getStatus()) {
                case BUFFER_OVERFLOW: {
                    out.ensureWritable(engine.getSession().getApplicationBufferSize());
                    continue block3;
                }
            }
            break;
        }
        return result;
    }

    private void runDelegatedTasks() {
        Runnable task;
        while ((task = this.engine.getDelegatedTask()) != null) {
            this.delegatedTaskExecutor.execute(task);
        }
    }

    private void setHandshakeSuccess() {
        if (this.handshakePromise.trySuccess(this.ctx.channel())) {
            this.ctx.fireUserEventTriggered(SslHandshakeCompletionEvent.SUCCESS);
        }
    }

    private void setHandshakeFailure(Throwable cause) {
        PendingWrite write;
        block3: {
            this.engine.closeOutbound();
            try {
                this.engine.closeInbound();
            }
            catch (SSLException e2) {
                String msg = e2.getMessage();
                if (msg != null && msg.contains("possible truncation attack")) break block3;
                logger.debug("SSLEngine.closeInbound() raised an exception.", e2);
            }
        }
        this.notifyHandshakeFailure(cause);
        while ((write = this.pendingUnencryptedWrites.poll()) != null) {
            write.failAndRecycle(cause);
        }
    }

    private void notifyHandshakeFailure(Throwable cause) {
        if (this.handshakePromise.tryFailure(cause)) {
            this.ctx.fireUserEventTriggered(new SslHandshakeCompletionEvent(cause));
            this.ctx.close();
        }
    }

    private void closeOutboundAndChannel(ChannelHandlerContext ctx, ChannelPromise promise, boolean disconnect) throws Exception {
        if (!ctx.channel().isActive()) {
            if (disconnect) {
                ctx.disconnect(promise);
            } else {
                ctx.close(promise);
            }
            return;
        }
        this.engine.closeOutbound();
        ChannelPromise closeNotifyFuture = ctx.newPromise();
        this.write(ctx, Unpooled.EMPTY_BUFFER, closeNotifyFuture);
        this.flush(ctx);
        this.safeClose(ctx, closeNotifyFuture, promise);
    }

    @Override
    public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
        this.ctx = ctx;
        if (ctx.channel().isActive()) {
            this.handshake0();
        }
    }

    private Future<Channel> handshake0() {
        final ScheduledFuture<?> timeoutFuture = this.handshakeTimeoutMillis > 0L ? this.ctx.executor().schedule(new Runnable(){

            @Override
            public void run() {
                if (SslHandler.this.handshakePromise.isDone()) {
                    return;
                }
                SslHandler.this.notifyHandshakeFailure(HANDSHAKE_TIMED_OUT);
            }
        }, this.handshakeTimeoutMillis, TimeUnit.MILLISECONDS) : null;
        this.handshakePromise.addListener(new GenericFutureListener<Future<Channel>>(){

            @Override
            public void operationComplete(Future<Channel> f2) throws Exception {
                if (timeoutFuture != null) {
                    timeoutFuture.cancel(false);
                }
            }
        });
        try {
            this.engine.beginHandshake();
            this.flushNonAppData0(this.ctx, false);
        }
        catch (Exception e2) {
            this.notifyHandshakeFailure(e2);
        }
        return this.handshakePromise;
    }

    @Override
    public void channelActive(final ChannelHandlerContext ctx) throws Exception {
        if (!this.startTls && this.engine.getUseClientMode()) {
            this.handshake0().addListener(new GenericFutureListener<Future<Channel>>(){

                @Override
                public void operationComplete(Future<Channel> future) throws Exception {
                    if (!future.isSuccess()) {
                        future.cause().printStackTrace();
                        ctx.close();
                    }
                }
            });
        }
        ctx.fireChannelActive();
    }

    private void safeClose(final ChannelHandlerContext ctx, ChannelFuture flushFuture, final ChannelPromise promise) {
        if (!ctx.channel().isActive()) {
            ctx.close(promise);
            return;
        }
        final ScheduledFuture<?> timeoutFuture = this.closeNotifyTimeoutMillis > 0L ? ctx.executor().schedule(new Runnable(){

            @Override
            public void run() {
                logger.warn(ctx.channel() + " last write attempt timed out." + " Force-closing the connection.");
                ctx.close(promise);
            }
        }, this.closeNotifyTimeoutMillis, TimeUnit.MILLISECONDS) : null;
        flushFuture.addListener(new ChannelFutureListener(){

            @Override
            public void operationComplete(ChannelFuture f2) throws Exception {
                if (timeoutFuture != null) {
                    timeoutFuture.cancel(false);
                }
                if (ctx.channel().isActive()) {
                    ctx.close(promise);
                }
            }
        });
    }

    static {
        SSLENGINE_CLOSED.setStackTrace(EmptyArrays.EMPTY_STACK_TRACE);
        HANDSHAKE_TIMED_OUT.setStackTrace(EmptyArrays.EMPTY_STACK_TRACE);
        CHANNEL_CLOSED.setStackTrace(EmptyArrays.EMPTY_STACK_TRACE);
    }

    private final class LazyChannelPromise
    extends DefaultPromise<Channel> {
        private LazyChannelPromise() {
        }

        @Override
        protected EventExecutor executor() {
            if (SslHandler.this.ctx == null) {
                throw new IllegalStateException();
            }
            return SslHandler.this.ctx.executor();
        }
    }
}

