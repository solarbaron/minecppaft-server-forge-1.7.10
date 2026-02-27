/*
 * Decompiled with CFR 0.152.
 */
public class gf
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private dh e;

    public gf() {
    }

    public gf(int n2, int n3, int n4, int n5, dh dh2) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = dh2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readShort();
        this.c = et2.readInt();
        this.d = et2.readUnsignedByte();
        this.e = et2.b();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeShort(this.b);
        et2.writeInt(this.c);
        et2.writeByte((byte)this.d);
        et2.a(this.e);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

