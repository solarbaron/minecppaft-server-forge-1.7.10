/*
 * Decompiled with CFR 0.152.
 */
import io.netty.buffer.ByteBuf;

public class gr
extends ft {
    private String a;
    private byte[] b;

    public gr() {
    }

    public gr(String string, ByteBuf byteBuf) {
        this(string, byteBuf.array());
    }

    public gr(String string, byte[] byArray) {
        this.a = string;
        this.b = byArray;
        if (byArray.length >= 0x100000) {
            throw new IllegalArgumentException("Payload may not be larger than 1048576 bytes");
        }
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(20);
        this.b = new byte[et2.readUnsignedShort()];
        et2.readBytes(this.b);
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeShort(this.b.length);
        et2.writeBytes(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

