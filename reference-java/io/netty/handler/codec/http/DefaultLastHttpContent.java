/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.handler.codec.http.DefaultHttpContent;
import io.netty.handler.codec.http.DefaultHttpHeaders;
import io.netty.handler.codec.http.HttpHeaders;
import io.netty.handler.codec.http.LastHttpContent;
import io.netty.util.internal.StringUtil;
import java.util.Map;

public class DefaultLastHttpContent
extends DefaultHttpContent
implements LastHttpContent {
    private final HttpHeaders trailingHeaders = new DefaultHttpHeaders(){

        @Override
        void validateHeaderName0(String name) {
            super.validateHeaderName0(name);
            if (name.equalsIgnoreCase("Content-Length") || name.equalsIgnoreCase("Transfer-Encoding") || name.equalsIgnoreCase("Trailer")) {
                throw new IllegalArgumentException("prohibited trailing header: " + name);
            }
        }
    };

    public DefaultLastHttpContent() {
        this(Unpooled.buffer(0));
    }

    public DefaultLastHttpContent(ByteBuf content) {
        super(content);
    }

    @Override
    public LastHttpContent copy() {
        DefaultLastHttpContent copy = new DefaultLastHttpContent(this.content().copy());
        copy.trailingHeaders().set(this.trailingHeaders());
        return copy;
    }

    @Override
    public LastHttpContent duplicate() {
        DefaultLastHttpContent copy = new DefaultLastHttpContent(this.content().duplicate());
        copy.trailingHeaders().set(this.trailingHeaders());
        return copy;
    }

    @Override
    public LastHttpContent retain(int increment) {
        super.retain(increment);
        return this;
    }

    @Override
    public LastHttpContent retain() {
        super.retain();
        return this;
    }

    @Override
    public HttpHeaders trailingHeaders() {
        return this.trailingHeaders;
    }

    @Override
    public String toString() {
        StringBuilder buf = new StringBuilder(super.toString());
        buf.append(StringUtil.NEWLINE);
        this.appendHeaders(buf);
        buf.setLength(buf.length() - StringUtil.NEWLINE.length());
        return buf.toString();
    }

    private void appendHeaders(StringBuilder buf) {
        for (Map.Entry e2 : this.trailingHeaders()) {
            buf.append((String)e2.getKey());
            buf.append(": ");
            buf.append((String)e2.getValue());
            buf.append(StringUtil.NEWLINE);
        }
    }
}

