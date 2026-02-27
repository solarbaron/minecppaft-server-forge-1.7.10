/*
 * Decompiled with CFR 0.152.
 */
public class jd
extends ft {
    protected double a;
    protected double b;
    protected double c;
    protected double d;
    protected float e;
    protected float f;
    protected boolean g;
    protected boolean h;
    protected boolean i;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.g = et2.readUnsignedByte() != 0;
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.g ? 1 : 0);
    }

    public double c() {
        return this.a;
    }

    public double d() {
        return this.b;
    }

    public double e() {
        return this.c;
    }

    public double f() {
        return this.d;
    }

    public float g() {
        return this.e;
    }

    public float h() {
        return this.f;
    }

    public boolean i() {
        return this.g;
    }

    public boolean j() {
        return this.h;
    }

    public boolean k() {
        return this.i;
    }

    public void a(boolean bl2) {
        this.h = bl2;
    }
}

