/*
 * Decompiled with CFR 0.152.
 */
public class iz
extends ft {
    private String a;
    private int b;
    private byte[] c;

    @Override
    public void a(et et2) {
        this.a = et2.c(20);
        this.b = et2.readShort();
        if (this.b > 0 && this.b < Short.MAX_VALUE) {
            this.c = new byte[this.b];
            et2.readBytes(this.c);
        }
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeShort((short)this.b);
        if (this.c != null) {
            et2.writeBytes(this.c);
        }
    }

    public void a(io io2) {
        io2.a(this);
    }

    public String c() {
        return this.a;
    }

    public byte[] e() {
        return this.c;
    }
}

