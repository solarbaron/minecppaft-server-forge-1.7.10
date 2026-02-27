/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.buffer.ByteBuf;
import io.netty.handler.codec.http.HttpObjectEncoder;
import io.netty.handler.codec.http.HttpRequest;
import io.netty.handler.codec.http.HttpResponse;

public class HttpResponseEncoder
extends HttpObjectEncoder<HttpResponse> {
    private static final byte[] CRLF = new byte[]{13, 10};

    @Override
    public boolean acceptOutboundMessage(Object msg) throws Exception {
        return super.acceptOutboundMessage(msg) && !(msg instanceof HttpRequest);
    }

    @Override
    protected void encodeInitialLine(ByteBuf buf, HttpResponse response) throws Exception {
        HttpResponseEncoder.encodeAscii(response.getProtocolVersion().toString(), buf);
        buf.writeByte(32);
        HttpResponseEncoder.encodeAscii(String.valueOf(response.getStatus().code()), buf);
        buf.writeByte(32);
        HttpResponseEncoder.encodeAscii(String.valueOf(response.getStatus().reasonPhrase()), buf);
        buf.writeBytes(CRLF);
    }
}

