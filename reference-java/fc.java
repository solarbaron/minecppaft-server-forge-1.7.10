/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.CorruptedFrameException;
import java.util.List;

public class fc
extends ByteToMessageDecoder {
    /*
     * WARNING - Removed try catching itself - possible behaviour change.
     */
    protected void decode(ChannelHandlerContext channelHandlerContext, ByteBuf byteBuf, List list) {
        byteBuf.markReaderIndex();
        byte[] byArray = new byte[3];
        for (int i2 = 0; i2 < byArray.length; ++i2) {
            if (!byteBuf.isReadable()) {
                byteBuf.resetReaderIndex();
                return;
            }
            byArray[i2] = byteBuf.readByte();
            if (byArray[i2] < 0) continue;
            et et2 = new et(Unpooled.wrappedBuffer(byArray));
            try {
                int n2 = et2.a();
                if (byteBuf.readableBytes() < n2) {
                    byteBuf.resetReaderIndex();
                    return;
                }
                list.add(byteBuf.readBytes(n2));
                return;
            }
            finally {
                et2.release();
            }
        }
        throw new CorruptedFrameException("length wider than 21-bit");
    }
}

