/*
 * Decompiled with CFR 0.152.
 */
public class ht
extends ft {
    private int a;
    private byte b;

    public ht() {
    }

    public ht(sa sa2, byte by2) {
        this.a = sa2.y();
        this.b = by2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, rot=%d", this.a, this.b);
    }
}

