/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.BiMap;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;
import java.io.IOException;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.MarkerManager;

public class fa
extends MessageToByteEncoder {
    private static final Logger a = LogManager.getLogger();
    private static final Marker b = MarkerManager.getMarker("PACKET_SENT", ej.b);
    private final eu c;

    public fa(eu eu2) {
        this.c = eu2;
    }

    protected void a(ChannelHandlerContext channelHandlerContext, ft ft2, ByteBuf byteBuf) {
        Integer n2 = (Integer)((BiMap)channelHandlerContext.channel().attr(ej.f).get()).inverse().get(ft2.getClass());
        if (a.isDebugEnabled()) {
            a.debug(b, "OUT: [{}:{}] {}[{}]", channelHandlerContext.channel().attr(ej.d).get(), n2, ft2.getClass().getName(), ft2.b());
        }
        if (n2 == null) {
            throw new IOException("Can't serialize unregistered packet");
        }
        et et2 = new et(byteBuf);
        et2.b(n2);
        ft2.b(et2);
        this.c.b(n2, et2.readableBytes());
    }

    protected /* synthetic */ void encode(ChannelHandlerContext channelHandlerContext, Object object, ByteBuf byteBuf) {
        this.a(channelHandlerContext, (ft)object, byteBuf);
    }
}

