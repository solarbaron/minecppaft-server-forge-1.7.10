/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.spdy.SpdyCodecUtil;
import io.netty.handler.codec.spdy.SpdyHeaderBlockRawDecoder;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;
import io.netty.handler.codec.spdy.SpdyProtocolException;
import java.util.zip.DataFormatException;
import java.util.zip.Inflater;

class SpdyHeaderBlockZlibDecoder
extends SpdyHeaderBlockRawDecoder {
    private final int version;
    private final byte[] out = new byte[8192];
    private final Inflater decompressor = new Inflater();
    private ByteBuf decompressed;

    public SpdyHeaderBlockZlibDecoder(int version, int maxHeaderSize) {
        super(version, maxHeaderSize);
        this.version = version;
    }

    @Override
    void decode(ByteBuf encoded, SpdyHeadersFrame frame) throws Exception {
        int numBytes;
        this.setInput(encoded);
        do {
            numBytes = this.decompress(frame);
        } while (!this.decompressed.isReadable() && numBytes > 0);
    }

    private void setInput(ByteBuf compressed) {
        byte[] in2 = new byte[compressed.readableBytes()];
        compressed.readBytes(in2);
        this.decompressor.setInput(in2);
    }

    private int decompress(SpdyHeadersFrame frame) throws Exception {
        if (this.decompressed == null) {
            this.decompressed = Unpooled.buffer(8192);
        }
        try {
            int numBytes = this.decompressor.inflate(this.out);
            if (numBytes == 0 && this.decompressor.needsDictionary()) {
                if (this.version < 3) {
                    this.decompressor.setDictionary(SpdyCodecUtil.SPDY2_DICT);
                } else {
                    this.decompressor.setDictionary(SpdyCodecUtil.SPDY_DICT);
                }
                numBytes = this.decompressor.inflate(this.out);
            }
            if (frame != null) {
                this.decompressed.writeBytes(this.out, 0, numBytes);
                super.decode(this.decompressed, frame);
                this.decompressed.discardReadBytes();
            }
            return numBytes;
        }
        catch (DataFormatException e2) {
            throw new SpdyProtocolException("Received invalid header block", e2);
        }
    }

    @Override
    public void reset() {
        this.decompressed = null;
        super.reset();
    }

    @Override
    public void end() {
        this.decompressed = null;
        this.decompressor.end();
        super.end();
    }
}

