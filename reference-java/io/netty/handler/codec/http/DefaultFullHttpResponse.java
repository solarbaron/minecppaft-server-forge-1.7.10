/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.http.DefaultHttpHeaders;
import io.netty.handler.codec.http.DefaultHttpResponse;
import io.netty.handler.codec.http.FullHttpResponse;
import io.netty.handler.codec.http.HttpHeaders;
import io.netty.handler.codec.http.HttpResponseStatus;
import io.netty.handler.codec.http.HttpVersion;

public class DefaultFullHttpResponse
extends DefaultHttpResponse
implements FullHttpResponse {
    private final ByteBuf content;
    private final HttpHeaders trailingHeaders = new DefaultHttpHeaders();

    public DefaultFullHttpResponse(HttpVersion version, HttpResponseStatus status) {
        this(version, status, Unpooled.buffer(0));
    }

    public DefaultFullHttpResponse(HttpVersion version, HttpResponseStatus status, ByteBuf content) {
        super(version, status);
        if (content == null) {
            throw new NullPointerException("content");
        }
        this.content = content;
    }

    @Override
    public HttpHeaders trailingHeaders() {
        return this.trailingHeaders;
    }

    @Override
    public ByteBuf content() {
        return this.content;
    }

    @Override
    public int refCnt() {
        return this.content.refCnt();
    }

    @Override
    public FullHttpResponse retain() {
        this.content.retain();
        return this;
    }

    @Override
    public FullHttpResponse retain(int increment) {
        this.content.retain(increment);
        return this;
    }

    @Override
    public boolean release() {
        return this.content.release();
    }

    @Override
    public boolean release(int decrement) {
        return this.content.release(decrement);
    }

    @Override
    public FullHttpResponse setProtocolVersion(HttpVersion version) {
        super.setProtocolVersion(version);
        return this;
    }

    @Override
    public FullHttpResponse setStatus(HttpResponseStatus status) {
        super.setStatus(status);
        return this;
    }

    @Override
    public FullHttpResponse copy() {
        DefaultFullHttpResponse copy = new DefaultFullHttpResponse(this.getProtocolVersion(), this.getStatus(), this.content().copy());
        copy.headers().set(this.headers());
        copy.trailingHeaders().set(this.trailingHeaders());
        return copy;
    }

    @Override
    public FullHttpResponse duplicate() {
        DefaultFullHttpResponse duplicate = new DefaultFullHttpResponse(this.getProtocolVersion(), this.getStatus(), this.content().duplicate());
        duplicate.headers().set(this.headers());
        duplicate.trailingHeaders().set(this.trailingHeaders());
        return duplicate;
    }
}

