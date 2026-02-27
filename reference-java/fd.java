/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;

public class fd
extends MessageToByteEncoder {
    protected void a(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, ByteBuf byteBuf2) {
        int n2 = byteBuf.readableBytes();
        int n3 = et.a(n2);
        if (n3 > 3) {
            throw new IllegalArgumentException("unable to fit " + n2 + " into " + 3);
        }
        et et2 = new et(byteBuf2);
        et2.ensureWritable(n3 + n2);
        et2.b(n2);
        et2.writeBytes(byteBuf, byteBuf.readerIndex(), n2);
    }

    protected /* synthetic */ void encode(ChannelHandlerContext channelHandlerContext, Object object, ByteBuf byteBuf) {
        this.a(channelHandlerContext, (ByteBuf)object, byteBuf);
    }
}

