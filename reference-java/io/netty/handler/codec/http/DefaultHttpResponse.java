/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.handler.codec.http.DefaultHttpMessage;
import io.netty.handler.codec.http.HttpResponse;
import io.netty.handler.codec.http.HttpResponseStatus;
import io.netty.handler.codec.http.HttpVersion;
import io.netty.util.internal.StringUtil;

public class DefaultHttpResponse
extends DefaultHttpMessage
implements HttpResponse {
    private HttpResponseStatus status;

    public DefaultHttpResponse(HttpVersion version, HttpResponseStatus status) {
        super(version);
        if (status == null) {
            throw new NullPointerException("status");
        }
        this.status = status;
    }

    @Override
    public HttpResponseStatus getStatus() {
        return this.status;
    }

    @Override
    public HttpResponse setStatus(HttpResponseStatus status) {
        if (status == null) {
            throw new NullPointerException("status");
        }
        this.status = status;
        return this;
    }

    @Override
    public HttpResponse setProtocolVersion(HttpVersion version) {
        super.setProtocolVersion(version);
        return this;
    }

    @Override
    public String toString() {
        StringBuilder buf = new StringBuilder();
        buf.append(this.getClass().getSimpleName());
        buf.append("(decodeResult: ");
        buf.append(this.getDecoderResult());
        buf.append(')');
        buf.append(StringUtil.NEWLINE);
        buf.append(this.getProtocolVersion().text());
        buf.append(' ');
        buf.append(this.getStatus().toString());
        buf.append(StringUtil.NEWLINE);
        this.appendHeaders(buf);
        buf.setLength(buf.length() - StringUtil.NEWLINE.length());
        return buf.toString();
    }
}

