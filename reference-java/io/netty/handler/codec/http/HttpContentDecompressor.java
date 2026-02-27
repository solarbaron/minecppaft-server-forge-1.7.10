/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.channel.embedded.EmbeddedChannel;
import io.netty.handler.codec.compression.ZlibCodecFactory;
import io.netty.handler.codec.compression.ZlibWrapper;
import io.netty.handler.codec.http.HttpContentDecoder;

public class HttpContentDecompressor
extends HttpContentDecoder {
    @Override
    protected EmbeddedChannel newContentDecoder(String contentEncoding) throws Exception {
        if ("gzip".equalsIgnoreCase(contentEncoding) || "x-gzip".equalsIgnoreCase(contentEncoding)) {
            return new EmbeddedChannel(ZlibCodecFactory.newZlibDecoder(ZlibWrapper.GZIP));
        }
        if ("deflate".equalsIgnoreCase(contentEncoding) || "x-deflate".equalsIgnoreCase(contentEncoding)) {
            return new EmbeddedChannel(ZlibCodecFactory.newZlibDecoder(ZlibWrapper.ZLIB_OR_NONE));
        }
        return null;
    }
}

