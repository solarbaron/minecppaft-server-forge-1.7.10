/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.handler.codec.DecoderException;
import io.netty.util.internal.RecyclableArrayList;
import io.netty.util.internal.StringUtil;
import java.util.List;

public abstract class ByteToMessageDecoder
extends ChannelInboundHandlerAdapter {
    ByteBuf cumulation;
    private boolean singleDecode;
    private boolean decodeWasNull;

    protected ByteToMessageDecoder() {
        if (this.getClass().isAnnotationPresent(ChannelHandler.Sharable.class)) {
            throw new IllegalStateException("@Sharable annotation is not allowed");
        }
    }

    public void setSingleDecode(boolean singleDecode) {
        this.singleDecode = singleDecode;
    }

    public boolean isSingleDecode() {
        return this.singleDecode;
    }

    protected int actualReadableBytes() {
        return this.internalBuffer().readableBytes();
    }

    protected ByteBuf internalBuffer() {
        if (this.cumulation != null) {
            return this.cumulation;
        }
        return Unpooled.EMPTY_BUFFER;
    }

    @Override
    public final void handlerRemoved(ChannelHandlerContext ctx) throws Exception {
        ByteBuf buf = this.internalBuffer();
        int readable = buf.readableBytes();
        if (buf.isReadable()) {
            ByteBuf bytes = buf.readBytes(readable);
            buf.release();
            ctx.fireChannelRead(bytes);
        }
        this.cumulation = null;
        ctx.fireChannelReadComplete();
        this.handlerRemoved0(ctx);
    }

    protected void handlerRemoved0(ChannelHandlerContext ctx) throws Exception {
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        block22: {
            RecyclableArrayList out = RecyclableArrayList.newInstance();
            try {
                if (msg instanceof ByteBuf) {
                    ByteBuf data = (ByteBuf)msg;
                    if (this.cumulation == null) {
                        this.cumulation = data;
                        try {
                            this.callDecode(ctx, this.cumulation, out);
                            break block22;
                        }
                        finally {
                            if (this.cumulation != null && !this.cumulation.isReadable()) {
                                this.cumulation.release();
                                this.cumulation = null;
                            }
                        }
                    }
                    try {
                        if (this.cumulation.writerIndex() > this.cumulation.maxCapacity() - data.readableBytes()) {
                            ByteBuf oldCumulation = this.cumulation;
                            this.cumulation = ctx.alloc().buffer(oldCumulation.readableBytes() + data.readableBytes());
                            this.cumulation.writeBytes(oldCumulation);
                            oldCumulation.release();
                        }
                        this.cumulation.writeBytes(data);
                        this.callDecode(ctx, this.cumulation, out);
                        break block22;
                    }
                    finally {
                        if (this.cumulation != null) {
                            if (!this.cumulation.isReadable()) {
                                this.cumulation.release();
                                this.cumulation = null;
                            } else {
                                this.cumulation.discardSomeReadBytes();
                            }
                        }
                        data.release();
                    }
                }
                out.add(msg);
            }
            catch (DecoderException e2) {
                throw e2;
            }
            catch (Throwable t2) {
                throw new DecoderException(t2);
            }
            finally {
                int size = out.size();
                if (size == 0) {
                    this.decodeWasNull = true;
                } else {
                    for (int i2 = 0; i2 < size; ++i2) {
                        ctx.fireChannelRead(out.get(i2));
                    }
                }
                out.recycle();
            }
        }
    }

    @Override
    public void channelReadComplete(ChannelHandlerContext ctx) throws Exception {
        if (this.decodeWasNull) {
            this.decodeWasNull = false;
            if (!ctx.channel().config().isAutoRead()) {
                ctx.read();
            }
        }
        ctx.fireChannelReadComplete();
    }

    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        RecyclableArrayList out = RecyclableArrayList.newInstance();
        try {
            if (this.cumulation != null) {
                this.callDecode(ctx, this.cumulation, out);
                this.decodeLast(ctx, this.cumulation, out);
            } else {
                this.decodeLast(ctx, Unpooled.EMPTY_BUFFER, out);
            }
        }
        catch (DecoderException e2) {
            throw e2;
        }
        catch (Exception e3) {
            throw new DecoderException(e3);
        }
        finally {
            if (this.cumulation != null) {
                this.cumulation.release();
                this.cumulation = null;
            }
            int size = out.size();
            for (int i2 = 0; i2 < size; ++i2) {
                ctx.fireChannelRead(out.get(i2));
            }
            ctx.fireChannelInactive();
            out.recycle();
        }
    }

    protected void callDecode(ChannelHandlerContext ctx, ByteBuf in2, List<Object> out) {
        try {
            while (in2.isReadable()) {
                int outSize = out.size();
                int oldInputLength = in2.readableBytes();
                this.decode(ctx, in2, out);
                if (!ctx.isRemoved()) {
                    if (outSize == out.size()) {
                        if (oldInputLength != in2.readableBytes()) continue;
                    } else {
                        if (oldInputLength == in2.readableBytes()) {
                            throw new DecoderException(StringUtil.simpleClassName(this.getClass()) + ".decode() did not read anything but decoded a message.");
                        }
                        if (!this.isSingleDecode()) continue;
                    }
                }
                break;
            }
        }
        catch (DecoderException e2) {
            throw e2;
        }
        catch (Throwable cause) {
            throw new DecoderException(cause);
        }
    }

    protected abstract void decode(ChannelHandlerContext var1, ByteBuf var2, List<Object> var3) throws Exception;

    protected void decodeLast(ChannelHandlerContext ctx, ByteBuf in2, List<Object> out) throws Exception {
        this.decode(ctx, in2, out);
    }
}

