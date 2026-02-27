/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;
import javax.crypto.Cipher;

public class ei
extends MessageToByteEncoder {
    private final eg a;

    public ei(Cipher cipher) {
        this.a = new eg(cipher);
    }

    protected void a(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, ByteBuf byteBuf2) {
        this.a.a(byteBuf, byteBuf2);
    }

    protected /* synthetic */ void encode(ChannelHandlerContext channelHandlerContext, Object object, ByteBuf byteBuf) {
        this.a(channelHandlerContext, (ByteBuf)object, byteBuf);
    }
}

