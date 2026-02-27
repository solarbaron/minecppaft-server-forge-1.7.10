/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToMessageDecoder;
import java.util.List;
import javax.crypto.Cipher;

public class eh
extends MessageToMessageDecoder {
    private final eg a;

    public eh(Cipher cipher) {
        this.a = new eg(cipher);
    }

    protected void a(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, List list) {
        list.add(this.a.a(channelHandlerContext, byteBuf));
    }

    protected /* synthetic */ void decode(ChannelHandlerContext channelHandlerContext, Object object, List list) {
        this.a(channelHandlerContext, (ByteBuf)object, list);
    }
}

