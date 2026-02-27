/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.spdy;

import io.netty.channel.CombinedChannelDuplexHandler;
import io.netty.handler.codec.spdy.SpdyHttpDecoder;
import io.netty.handler.codec.spdy.SpdyHttpEncoder;

public final class SpdyHttpCodec
extends CombinedChannelDuplexHandler<SpdyHttpDecoder, SpdyHttpEncoder> {
    public SpdyHttpCodec(int version, int maxContentLength) {
        super(new SpdyHttpDecoder(version, maxContentLength), new SpdyHttpEncoder(version));
    }
}

