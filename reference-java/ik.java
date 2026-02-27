/*
 * Decompiled with CFR 0.152.
 */
public class ik
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private byte e;
    private byte f;

    public ik() {
    }

    public ik(sa sa2) {
        this.a = sa2.y();
        this.b = qh.c(sa2.s * 32.0);
        this.c = qh.c(sa2.t * 32.0);
        this.d = qh.c(sa2.u * 32.0);
        this.e = (byte)(sa2.y * 256.0f / 360.0f);
        this.f = (byte)(sa2.z * 256.0f / 360.0f);
    }

    public ik(int n2, int n3, int n4, int n5, byte by2, byte by3) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = by2;
        this.f = by3;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readByte();
        this.f = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeByte(this.e);
        et2.writeByte(this.f);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

