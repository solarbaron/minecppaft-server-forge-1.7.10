/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.buffer.ByteBuf;
import io.netty.handler.codec.spdy.SpdyHeaderBlockZlibDecoder;
import io.netty.handler.codec.spdy.SpdyHeadersFrame;

abstract class SpdyHeaderBlockDecoder {
    SpdyHeaderBlockDecoder() {
    }

    static SpdyHeaderBlockDecoder newInstance(int version, int maxHeaderSize) {
        return new SpdyHeaderBlockZlibDecoder(version, maxHeaderSize);
    }

    abstract void decode(ByteBuf var1, SpdyHeadersFrame var2) throws Exception;

    abstract void reset();

    abstract void end();
}

