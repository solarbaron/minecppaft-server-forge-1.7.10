/*
 * Decompiled with CFR 0.152.
 */
public class gc
extends ft {
    private int a;
    private int b;

    public gc() {
    }

    public gc(sa sa2, int n2) {
        this.a = sa2.y();
        this.b = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeByte(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%d", this.a, this.b);
    }
}

