/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.rtsp;

import io.netty.handler.codec.http.DefaultHttpResponse;
import io.netty.handler.codec.http.HttpMessage;
import io.netty.handler.codec.http.HttpResponseStatus;
import io.netty.handler.codec.rtsp.RtspObjectDecoder;
import io.netty.handler.codec.rtsp.RtspVersions;

public class RtspResponseDecoder
extends RtspObjectDecoder {
    private static final HttpResponseStatus UNKNOWN_STATUS = new HttpResponseStatus(999, "Unknown");

    public RtspResponseDecoder() {
    }

    public RtspResponseDecoder(int maxInitialLineLength, int maxHeaderSize, int maxContentLength) {
        super(maxInitialLineLength, maxHeaderSize, maxContentLength);
    }

    @Override
    protected HttpMessage createMessage(String[] initialLine) throws Exception {
        return new DefaultHttpResponse(RtspVersions.valueOf(initialLine[0]), new HttpResponseStatus(Integer.valueOf(initialLine[1]), initialLine[2]));
    }

    @Override
    protected HttpMessage createInvalidMessage() {
        return new DefaultHttpResponse(RtspVersions.RTSP_1_0, UNKNOWN_STATUS);
    }

    @Override
    protected boolean isDecodingRequest() {
        return false;
    }
}

