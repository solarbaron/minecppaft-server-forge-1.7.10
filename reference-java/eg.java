/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import javax.crypto.Cipher;

public class eg {
    private final Cipher a;
    private byte[] b = new byte[0];
    private byte[] c = new byte[0];

    protected eg(Cipher cipher) {
        this.a = cipher;
    }

    private byte[] a(ByteBuf byteBuf) {
        int n2 = byteBuf.readableBytes();
        if (this.b.length < n2) {
            this.b = new byte[n2];
        }
        byteBuf.readBytes(this.b, 0, n2);
        return this.b;
    }

    protected ByteBuf a(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf) {
        int n2 = byteBuf.readableBytes();
        byte[] byArray = this.a(byteBuf);
        ByteBuf byteBuf2 = channelHandlerContext.alloc().heapBuffer(this.a.getOutputSize(n2));
        byteBuf2.writerIndex(this.a.update(byArray, 0, n2, byteBuf2.array(), byteBuf2.arrayOffset()));
        return byteBuf2;
    }

    protected void a(ByteBuf byteBuf, ByteBuf byteBuf2) {
        int n2 = byteBuf.readableBytes();
        byte[] byArray = this.a(byteBuf);
        int n3 = this.a.getOutputSize(n2);
        if (this.c.length < n3) {
            this.c = new byte[n3];
        }
        byteBuf2.writeBytes(this.c, 0, this.a.update(byArray, 0, n2, this.c));
    }
}

