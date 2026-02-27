/*
 * Decompiled with CFR 0.152.
 */
package io.netty.handler.codec.http.websocketx;

import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelFutureListener;
import io.netty.channel.ChannelHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.ChannelPromise;
import io.netty.handler.codec.http.FullHttpRequest;
import io.netty.handler.codec.http.FullHttpResponse;
import io.netty.handler.codec.http.HttpContentCompressor;
import io.netty.handler.codec.http.HttpHeaders;
import io.netty.handler.codec.http.HttpObjectAggregator;
import io.netty.handler.codec.http.HttpRequestDecoder;
import io.netty.handler.codec.http.HttpResponseEncoder;
import io.netty.handler.codec.http.HttpServerCodec;
import io.netty.handler.codec.http.websocketx.CloseWebSocketFrame;
import io.netty.handler.codec.http.websocketx.WebSocketFrameDecoder;
import io.netty.handler.codec.http.websocketx.WebSocketFrameEncoder;
import io.netty.handler.codec.http.websocketx.WebSocketVersion;
import io.netty.util.internal.EmptyArrays;
import io.netty.util.internal.StringUtil;
import io.netty.util.internal.logging.InternalLogger;
import io.netty.util.internal.logging.InternalLoggerFactory;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;

public abstract class WebSocketServerHandshaker {
    protected static final InternalLogger logger = InternalLoggerFactory.getInstance(WebSocketServerHandshaker.class);
    private final String uri;
    private final String[] subprotocols;
    private final WebSocketVersion version;
    private final int maxFramePayloadLength;
    private String selectedSubprotocol;
    public static final String SUB_PROTOCOL_WILDCARD = "*";

    protected WebSocketServerHandshaker(WebSocketVersion version, String uri, String subprotocols, int maxFramePayloadLength) {
        this.version = version;
        this.uri = uri;
        if (subprotocols != null) {
            String[] subprotocolArray = StringUtil.split(subprotocols, ',');
            for (int i2 = 0; i2 < subprotocolArray.length; ++i2) {
                subprotocolArray[i2] = subprotocolArray[i2].trim();
            }
            this.subprotocols = subprotocolArray;
        } else {
            this.subprotocols = EmptyArrays.EMPTY_STRINGS;
        }
        this.maxFramePayloadLength = maxFramePayloadLength;
    }

    public String uri() {
        return this.uri;
    }

    public Set<String> subprotocols() {
        LinkedHashSet<String> ret = new LinkedHashSet<String>();
        Collections.addAll(ret, this.subprotocols);
        return ret;
    }

    public WebSocketVersion version() {
        return this.version;
    }

    public int maxFramePayloadLength() {
        return this.maxFramePayloadLength;
    }

    public ChannelFuture handshake(Channel channel, FullHttpRequest req) {
        return this.handshake(channel, req, null, channel.newPromise());
    }

    public final ChannelFuture handshake(Channel channel, FullHttpRequest req, HttpHeaders responseHeaders, final ChannelPromise promise) {
        if (logger.isDebugEnabled()) {
            logger.debug(String.format("%s WS Version %s server handshake", new Object[]{channel, this.version()}));
        }
        FullHttpResponse response = this.newHandshakeResponse(req, responseHeaders);
        channel.writeAndFlush(response).addListener(new ChannelFutureListener(){

            @Override
            public void operationComplete(ChannelFuture future) throws Exception {
                if (future.isSuccess()) {
                    ChannelHandlerContext ctx;
                    ChannelPipeline p2 = future.channel().pipeline();
                    if (p2.get(HttpObjectAggregator.class) != null) {
                        p2.remove(HttpObjectAggregator.class);
                    }
                    if (p2.get(HttpContentCompressor.class) != null) {
                        p2.remove(HttpContentCompressor.class);
                    }
                    if ((ctx = p2.context(HttpRequestDecoder.class)) == null) {
                        ctx = p2.context(HttpServerCodec.class);
                        if (ctx == null) {
                            promise.setFailure(new IllegalStateException("No HttpDecoder and no HttpServerCodec in the pipeline"));
                            return;
                        }
                        p2.addBefore(ctx.name(), "wsdecoder", WebSocketServerHandshaker.this.newWebsocketDecoder());
                        p2.replace(ctx.name(), "wsencoder", (ChannelHandler)WebSocketServerHandshaker.this.newWebSocketEncoder());
                    } else {
                        p2.replace(ctx.name(), "wsdecoder", (ChannelHandler)WebSocketServerHandshaker.this.newWebsocketDecoder());
                        p2.replace(HttpResponseEncoder.class, "wsencoder", (ChannelHandler)WebSocketServerHandshaker.this.newWebSocketEncoder());
                    }
                    promise.setSuccess();
                } else {
                    promise.setFailure(future.cause());
                }
            }
        });
        return promise;
    }

    protected abstract FullHttpResponse newHandshakeResponse(FullHttpRequest var1, HttpHeaders var2);

    public ChannelFuture close(Channel channel, CloseWebSocketFrame frame) {
        if (channel == null) {
            throw new NullPointerException("channel");
        }
        return this.close(channel, frame, channel.newPromise());
    }

    public ChannelFuture close(Channel channel, CloseWebSocketFrame frame, ChannelPromise promise) {
        if (channel == null) {
            throw new NullPointerException("channel");
        }
        return channel.writeAndFlush(frame, promise).addListener(ChannelFutureListener.CLOSE);
    }

    protected String selectSubprotocol(String requestedSubprotocols) {
        String[] requestedSubprotocolArray;
        if (requestedSubprotocols == null || this.subprotocols.length == 0) {
            return null;
        }
        for (String p2 : requestedSubprotocolArray = StringUtil.split(requestedSubprotocols, ',')) {
            String requestedSubprotocol = p2.trim();
            for (String supportedSubprotocol : this.subprotocols) {
                if (!SUB_PROTOCOL_WILDCARD.equals(supportedSubprotocol) && !requestedSubprotocol.equals(supportedSubprotocol)) continue;
                this.selectedSubprotocol = requestedSubprotocol;
                return requestedSubprotocol;
            }
        }
        return null;
    }

    public String selectedSubprotocol() {
        return this.selectedSubprotocol;
    }

    protected abstract WebSocketFrameDecoder newWebsocketDecoder();

    protected abstract WebSocketFrameEncoder newWebSocketEncoder();
}

