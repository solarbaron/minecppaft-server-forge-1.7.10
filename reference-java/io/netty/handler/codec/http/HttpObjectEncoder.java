/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.FileRegion;
import io.netty.handler.codec.MessageToMessageEncoder;
import io.netty.handler.codec.http.HttpContent;
import io.netty.handler.codec.http.HttpHeaders;
import io.netty.handler.codec.http.HttpMessage;
import io.netty.handler.codec.http.HttpObject;
import io.netty.handler.codec.http.LastHttpContent;
import io.netty.util.CharsetUtil;
import java.util.List;
import java.util.Map;

public abstract class HttpObjectEncoder<H extends HttpMessage>
extends MessageToMessageEncoder<Object> {
    private static final byte[] CRLF = new byte[]{13, 10};
    private static final byte[] ZERO_CRLF = new byte[]{48, 13, 10};
    private static final byte[] ZERO_CRLF_CRLF = new byte[]{48, 13, 10, 13, 10};
    private static final byte[] HEADER_SEPARATOR = new byte[]{58, 32};
    private static final ByteBuf CRLF_BUF = Unpooled.unreleasableBuffer(Unpooled.directBuffer(CRLF.length).writeBytes(CRLF));
    private static final ByteBuf ZERO_CRLF_CRLF_BUF = Unpooled.unreleasableBuffer(Unpooled.directBuffer(ZERO_CRLF_CRLF.length).writeBytes(ZERO_CRLF_CRLF));
    private static final int ST_INIT = 0;
    private static final int ST_CONTENT_NON_CHUNK = 1;
    private static final int ST_CONTENT_CHUNK = 2;
    private int state = 0;

    @Override
    protected void encode(ChannelHandlerContext ctx, Object msg, List<Object> out) throws Exception {
        if (msg instanceof HttpMessage) {
            if (this.state != 0) {
                throw new IllegalStateException("unexpected message type: " + msg.getClass().getSimpleName());
            }
            HttpMessage m2 = (HttpMessage)msg;
            ByteBuf buf = ctx.alloc().buffer();
            this.encodeInitialLine(buf, m2);
            HttpObjectEncoder.encodeHeaders(buf, m2.headers());
            buf.writeBytes(CRLF);
            out.add(buf);
            int n2 = this.state = HttpHeaders.isTransferEncodingChunked(m2) ? 2 : 1;
        }
        if (msg instanceof HttpContent || msg instanceof ByteBuf || msg instanceof FileRegion) {
            if (this.state == 0) {
                throw new IllegalStateException("unexpected message type: " + msg.getClass().getSimpleName());
            }
            int contentLength = HttpObjectEncoder.contentLength(msg);
            if (this.state == 1) {
                if (contentLength > 0) {
                    out.add(HttpObjectEncoder.encodeAndRetain(msg));
                } else {
                    out.add(Unpooled.EMPTY_BUFFER);
                }
                if (msg instanceof LastHttpContent) {
                    this.state = 0;
                }
            } else if (this.state == 2) {
                this.encodeChunkedContent(ctx, msg, contentLength, out);
            } else {
                throw new Error();
            }
        }
    }

    private void encodeChunkedContent(ChannelHandlerContext ctx, Object msg, int contentLength, List<Object> out) {
        ByteBuf buf;
        if (contentLength > 0) {
            byte[] length = Integer.toHexString(contentLength).getBytes(CharsetUtil.US_ASCII);
            buf = ctx.alloc().buffer(length.length + 2);
            buf.writeBytes(length);
            buf.writeBytes(CRLF);
            out.add(buf);
            out.add(HttpObjectEncoder.encodeAndRetain(msg));
            out.add(CRLF_BUF.duplicate());
        }
        if (msg instanceof LastHttpContent) {
            HttpHeaders headers = ((LastHttpContent)msg).trailingHeaders();
            if (headers.isEmpty()) {
                out.add(ZERO_CRLF_CRLF_BUF.duplicate());
            } else {
                buf = ctx.alloc().buffer();
                buf.writeBytes(ZERO_CRLF);
                HttpObjectEncoder.encodeHeaders(buf, headers);
                buf.writeBytes(CRLF);
                out.add(buf);
            }
            this.state = 0;
        } else if (contentLength == 0) {
            out.add(Unpooled.EMPTY_BUFFER);
        }
    }

    @Override
    public boolean acceptOutboundMessage(Object msg) throws Exception {
        return msg instanceof HttpObject || msg instanceof ByteBuf || msg instanceof FileRegion;
    }

    private static Object encodeAndRetain(Object msg) {
        if (msg instanceof ByteBuf) {
            return ((ByteBuf)msg).retain();
        }
        if (msg instanceof HttpContent) {
            return ((HttpContent)msg).content().retain();
        }
        if (msg instanceof FileRegion) {
            return ((FileRegion)msg).retain();
        }
        throw new IllegalStateException("unexpected message type: " + msg.getClass().getSimpleName());
    }

    private static int contentLength(Object msg) {
        if (msg instanceof HttpContent) {
            return ((HttpContent)msg).content().readableBytes();
        }
        if (msg instanceof ByteBuf) {
            return ((ByteBuf)msg).readableBytes();
        }
        if (msg instanceof FileRegion) {
            return (int)((FileRegion)msg).count();
        }
        throw new IllegalStateException("unexpected message type: " + msg.getClass().getSimpleName());
    }

    private static void encodeHeaders(ByteBuf buf, HttpHeaders headers) {
        for (Map.Entry h2 : headers) {
            HttpObjectEncoder.encodeHeader(buf, (String)h2.getKey(), (String)h2.getValue());
        }
    }

    private static void encodeHeader(ByteBuf buf, String header, String value) {
        HttpObjectEncoder.encodeAscii(header, buf);
        buf.writeBytes(HEADER_SEPARATOR);
        HttpObjectEncoder.encodeAscii(value, buf);
        buf.writeBytes(CRLF);
    }

    protected static void encodeAscii(String s2, ByteBuf buf) {
        for (int i2 = 0; i2 < s2.length(); ++i2) {
            buf.writeByte(s2.charAt(i2));
        }
    }

    protected abstract void encodeInitialLine(ByteBuf var1, H var2) throws Exception;
}

