/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.handler.codec.http.DefaultHttpResponse;
import io.netty.handler.codec.http.HttpMessage;
import io.netty.handler.codec.http.HttpObjectDecoder;
import io.netty.handler.codec.http.HttpResponseStatus;
import io.netty.handler.codec.http.HttpVersion;

public class HttpResponseDecoder
extends HttpObjectDecoder {
    private static final HttpResponseStatus UNKNOWN_STATUS = new HttpResponseStatus(999, "Unknown");

    public HttpResponseDecoder() {
    }

    public HttpResponseDecoder(int maxInitialLineLength, int maxHeaderSize, int maxChunkSize) {
        super(maxInitialLineLength, maxHeaderSize, maxChunkSize, true);
    }

    @Override
    protected HttpMessage createMessage(String[] initialLine) {
        return new DefaultHttpResponse(HttpVersion.valueOf(initialLine[0]), new HttpResponseStatus(Integer.valueOf(initialLine[1]), initialLine[2]));
    }

    @Override
    protected HttpMessage createInvalidMessage() {
        return new DefaultHttpResponse(HttpVersion.HTTP_1_0, UNKNOWN_STATUS);
    }

    @Override
    protected boolean isDecodingRequest() {
        return false;
    }
}

