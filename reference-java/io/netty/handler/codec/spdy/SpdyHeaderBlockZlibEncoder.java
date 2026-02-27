/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.spdy.SpdyCodecUtil;
import io.netty.handler.codec.spdy.SpdyHeaderBlockRawEncoder;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;
import java.util.zip.Deflater;

class SpdyHeaderBlockZlibEncoder
extends SpdyHeaderBlockRawEncoder {
    private final byte[] out = new byte[8192];
    private final Deflater compressor;
    private boolean finished;

    public SpdyHeaderBlockZlibEncoder(int version, int compressionLevel) {
        super(version);
        if (compressionLevel < 0 || compressionLevel > 9) {
            throw new IllegalArgumentException("compressionLevel: " + compressionLevel + " (expected: 0-9)");
        }
        this.compressor = new Deflater(compressionLevel);
        if (version < 3) {
            this.compressor.setDictionary(SpdyCodecUtil.SPDY2_DICT);
        } else {
            this.compressor.setDictionary(SpdyCodecUtil.SPDY_DICT);
        }
    }

    private void setInput(ByteBuf decompressed) {
        byte[] in2 = new byte[decompressed.readableBytes()];
        decompressed.readBytes(in2);
        this.compressor.setInput(in2);
    }

    private void encode(ByteBuf compressed) {
        int numBytes = this.out.length;
        while (numBytes == this.out.length) {
            numBytes = this.compressor.deflate(this.out, 0, this.out.length, 2);
            compressed.writeBytes(this.out, 0, numBytes);
        }
    }

    @Override
    public ByteBuf encode(ChannelHandlerContext ctx, SpdyHeadersFrame frame) throws Exception {
        if (frame == null) {
            throw new IllegalArgumentException("frame");
        }
        if (this.finished) {
            return Unpooled.EMPTY_BUFFER;
        }
        ByteBuf decompressed = super.encode(ctx, frame);
        if (decompressed.readableBytes() == 0) {
            return Unpooled.EMPTY_BUFFER;
        }
        ByteBuf compressed = ctx.alloc().buffer();
        this.setInput(decompressed);
        this.encode(compressed);
        return compressed;
    }

    @Override
    public void end() {
        if (this.finished) {
            return;
        }
        this.finished = true;
        this.compressor.end();
        super.end();
    }
}

