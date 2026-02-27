/*
 * Decompiled with CFR 0.152.
 */
public class hx
extends ft {
    private int a;
    private int b;
    private int c;

    public hx() {
    }

    public hx(int n2, sa sa2, sa sa3) {
        this.a = n2;
        this.b = sa2.y();
        this.c = sa3 != null ? sa3.y() : -1;
    }

    @Override
    public void a(et et2) {
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.a = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeByte(this.a);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

