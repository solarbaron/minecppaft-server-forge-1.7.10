/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.BiMap;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import java.io.IOException;
import java.util.List;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.MarkerManager;

public class ez
extends ByteToMessageDecoder {
    private static final Logger a = LogManager.getLogger();
    private static final Marker b = MarkerManager.getMarker("PACKET_RECEIVED", ej.b);
    private final eu c;

    public ez(eu eu2) {
        this.c = eu2;
    }

    protected void decode(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, List list) {
        int n2 = byteBuf.readableBytes();
        if (n2 == 0) {
            return;
        }
        et et2 = new et(byteBuf);
        int n3 = et2.a();
        ft ft2 = ft.a((BiMap)channelHandlerContext.channel().attr(ej.e).get(), n3);
        if (ft2 == null) {
            throw new IOException("Bad packet id " + n3);
        }
        ft2.a(et2);
        if (et2.readableBytes() > 0) {
            throw new IOException("Packet was larger than I expected, found " + et2.readableBytes() + " bytes extra whilst reading packet " + n3);
        }
        list.add(ft2);
        this.c.a(n3, n2);
        if (a.isDebugEnabled()) {
            a.debug(b, " IN: [{}:{}] {}[{}]", channelHandlerContext.channel().attr(ej.d).get(), n3, ft2.getClass().getName(), ft2.b());
        }
    }
}

