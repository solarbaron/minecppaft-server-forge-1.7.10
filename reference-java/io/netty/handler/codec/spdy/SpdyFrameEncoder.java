/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;
import io.netty.handler.codec.UnsupportedMessageTypeException;
import io.netty.handler.codec.spdy.SpdyDataFrame;
import io.netty.handler.codec.spdy.SpdyFrame;
import io.netty.handler.codec.spdy.SpdyGoAwayFrame;
import io.netty.handler.codec.spdy.SpdyHeaderBlockEncoder;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;
import io.netty.handler.codec.spdy.SpdyPingFrame;
import io.netty.handler.codec.spdy.SpdyRstStreamFrame;
import io.netty.handler.codec.spdy.SpdySettingsFrame;
import io.netty.handler.codec.spdy.SpdySynReplyFrame;
import io.netty.handler.codec.spdy.SpdySynStreamFrame;
import io.netty.handler.codec.spdy.SpdyWindowUpdateFrame;
import java.util.Set;

public class SpdyFrameEncoder
extends MessageToByteEncoder<SpdyFrame> {
    private final int version;
    private final SpdyHeaderBlockEncoder headerBlockEncoder;

    public SpdyFrameEncoder(int version) {
        this(version, 6, 15, 8);
    }

    public SpdyFrameEncoder(int version, int compressionLevel, int windowBits, int memLevel) {
        this(version, SpdyHeaderBlockEncoder.newInstance(version, compressionLevel, windowBits, memLevel));
    }

    protected SpdyFrameEncoder(int version, SpdyHeaderBlockEncoder headerBlockEncoder) {
        if (version < 2 || version > 3) {
            throw new IllegalArgumentException("unknown version: " + version);
        }
        this.version = version;
        this.headerBlockEncoder = headerBlockEncoder;
    }

    @Override
    public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
        ctx.channel().closeFuture().addListener(new ChannelFutureListener(){

            @Override
            public void operationComplete(ChannelFuture future) throws Exception {
                SpdyFrameEncoder.this.headerBlockEncoder.end();
            }
        });
    }

    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    @Override
    protected void encode(ChannelHandlerContext ctx, SpdyFrame msg, ByteBuf out) throws Exception {
        if (msg instanceof SpdyDataFrame) {
            SpdyDataFrame spdyDataFrame = (SpdyDataFrame)msg;
            ByteBuf data = spdyDataFrame.content();
            int flags = spdyDataFrame.isLast() ? 1 : 0;
            out.ensureWritable(8 + data.readableBytes());
            out.writeInt(spdyDataFrame.getStreamId() & Integer.MAX_VALUE);
            out.writeByte(flags);
            out.writeMedium(data.readableBytes());
            out.writeBytes(data, data.readerIndex(), data.readableBytes());
        } else if (msg instanceof SpdySynStreamFrame) {
            SpdySynStreamFrame spdySynStreamFrame = (SpdySynStreamFrame)msg;
            ByteBuf data = this.headerBlockEncoder.encode(ctx, spdySynStreamFrame);
            try {
                int flags;
                int n2 = flags = spdySynStreamFrame.isLast() ? 1 : 0;
                if (spdySynStreamFrame.isUnidirectional()) {
                    flags = (byte)(flags | 2);
                }
                int headerBlockLength = data.readableBytes();
                int length = this.version < 3 ? (headerBlockLength == 0 ? 12 : 10 + headerBlockLength) : 10 + headerBlockLength;
                out.ensureWritable(8 + length);
                out.writeShort(this.version | 0x8000);
                out.writeShort(1);
                out.writeByte(flags);
                out.writeMedium(length);
                out.writeInt(spdySynStreamFrame.getStreamId());
                out.writeInt(spdySynStreamFrame.getAssociatedToStreamId());
                if (this.version < 3) {
                    int priority = spdySynStreamFrame.getPriority();
                    if (priority > 3) {
                        priority = 3;
                    }
                    out.writeShort((priority & 0xFF) << 14);
                } else {
                    out.writeShort((spdySynStreamFrame.getPriority() & 0xFF) << 13);
                }
                if (this.version < 3 && data.readableBytes() == 0) {
                    out.writeShort(0);
                }
                out.writeBytes(data, data.readerIndex(), headerBlockLength);
            }
            finally {
                data.release();
            }
        } else if (msg instanceof SpdySynReplyFrame) {
            SpdySynReplyFrame spdySynReplyFrame = (SpdySynReplyFrame)msg;
            ByteBuf data = this.headerBlockEncoder.encode(ctx, spdySynReplyFrame);
            try {
                int flags = spdySynReplyFrame.isLast() ? 1 : 0;
                int headerBlockLength = data.readableBytes();
                int length = this.version < 3 ? (headerBlockLength == 0 ? 8 : 6 + headerBlockLength) : 4 + headerBlockLength;
                out.ensureWritable(8 + length);
                out.writeShort(this.version | 0x8000);
                out.writeShort(2);
                out.writeByte(flags);
                out.writeMedium(length);
                out.writeInt(spdySynReplyFrame.getStreamId());
                if (this.version < 3) {
                    if (headerBlockLength == 0) {
                        out.writeInt(0);
                    } else {
                        out.writeShort(0);
                    }
                }
                out.writeBytes(data, data.readerIndex(), headerBlockLength);
            }
            finally {
                data.release();
            }
        } else if (msg instanceof SpdyRstStreamFrame) {
            SpdyRstStreamFrame spdyRstStreamFrame = (SpdyRstStreamFrame)msg;
            out.ensureWritable(16);
            out.writeShort(this.version | 0x8000);
            out.writeShort(3);
            out.writeInt(8);
            out.writeInt(spdyRstStreamFrame.getStreamId());
            out.writeInt(spdyRstStreamFrame.getStatus().getCode());
        } else if (msg instanceof SpdySettingsFrame) {
            SpdySettingsFrame spdySettingsFrame = (SpdySettingsFrame)msg;
            int flags = spdySettingsFrame.clearPreviouslyPersistedSettings() ? 1 : 0;
            Set<Integer> IDs = spdySettingsFrame.getIds();
            int numEntries = IDs.size();
            int length = 4 + numEntries * 8;
            out.ensureWritable(8 + length);
            out.writeShort(this.version | 0x8000);
            out.writeShort(4);
            out.writeByte(flags);
            out.writeMedium(length);
            out.writeInt(numEntries);
            for (Integer ID : IDs) {
                int id2 = ID;
                byte ID_flags = 0;
                if (spdySettingsFrame.isPersistValue(id2)) {
                    ID_flags = (byte)(ID_flags | 1);
                }
                if (spdySettingsFrame.isPersisted(id2)) {
                    ID_flags = (byte)(ID_flags | 2);
                }
                if (this.version < 3) {
                    out.writeByte(id2 & 0xFF);
                    out.writeByte(id2 >> 8 & 0xFF);
                    out.writeByte(id2 >> 16 & 0xFF);
                    out.writeByte(ID_flags);
                } else {
                    out.writeByte(ID_flags);
                    out.writeMedium(id2);
                }
                out.writeInt(spdySettingsFrame.getValue(id2));
            }
        } else if (msg instanceof SpdyPingFrame) {
            SpdyPingFrame spdyPingFrame = (SpdyPingFrame)msg;
            out.ensureWritable(12);
            out.writeShort(this.version | 0x8000);
            out.writeShort(6);
            out.writeInt(4);
            out.writeInt(spdyPingFrame.getId());
        } else if (msg instanceof SpdyGoAwayFrame) {
            SpdyGoAwayFrame spdyGoAwayFrame = (SpdyGoAwayFrame)msg;
            int length = this.version < 3 ? 4 : 8;
            out.ensureWritable(8 + length);
            out.writeShort(this.version | 0x8000);
            out.writeShort(7);
            out.writeInt(length);
            out.writeInt(spdyGoAwayFrame.getLastGoodStreamId());
            if (this.version >= 3) {
                out.writeInt(spdyGoAwayFrame.getStatus().getCode());
            }
        } else if (msg instanceof SpdyHeadersFrame) {
            SpdyHeadersFrame spdyHeadersFrame = (SpdyHeadersFrame)msg;
            ByteBuf data = this.headerBlockEncoder.encode(ctx, spdyHeadersFrame);
            try {
                int flags = spdyHeadersFrame.isLast() ? 1 : 0;
                int headerBlockLength = data.readableBytes();
                int length = this.version < 3 ? (headerBlockLength == 0 ? 4 : 6 + headerBlockLength) : 4 + headerBlockLength;
                out.ensureWritable(8 + length);
                out.writeShort(this.version | 0x8000);
                out.writeShort(8);
                out.writeByte(flags);
                out.writeMedium(length);
                out.writeInt(spdyHeadersFrame.getStreamId());
                if (this.version < 3 && headerBlockLength != 0) {
                    out.writeShort(0);
                }
                out.writeBytes(data, data.readerIndex(), headerBlockLength);
            }
            finally {
                data.release();
            }
        } else if (msg instanceof SpdyWindowUpdateFrame) {
            SpdyWindowUpdateFrame spdyWindowUpdateFrame = (SpdyWindowUpdateFrame)msg;
            out.ensureWritable(16);
            out.writeShort(this.version | 0x8000);
            out.writeShort(9);
            out.writeInt(8);
            out.writeInt(spdyWindowUpdateFrame.getStreamId());
            out.writeInt(spdyWindowUpdateFrame.getDeltaWindowSize());
        } else {
            throw new UnsupportedMessageTypeException(msg, new Class[0]);
        }
    }
}

