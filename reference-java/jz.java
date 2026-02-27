/*
 * Decompiled with CFR 0.152.
 */
public class jz
extends ft {
    private long a;

    public jz() {
    }

    public jz(long l2) {
        this.a = l2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readLong();
    }

    @Override
    public void b(et et2) {
        et2.writeLong(this.a);
    }

    public void a(jy jy2) {
        jy2.a(this);
    }

    @Override
    public boolean a() {
        return true;
    }
}

