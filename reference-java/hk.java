/*
 * Decompiled with CFR 0.152.
 */
public class hk
extends ft {
    private boolean a;
    private boolean b;
    private boolean c;
    private boolean d;
    private float e;
    private float f;

    public hk() {
    }

    public hk(yw yw2) {
        this.a(yw2.a);
        this.b(yw2.b);
        this.c(yw2.c);
        this.d(yw2.d);
        this.a(yw2.a());
        this.b(yw2.b());
    }

    @Override
    public void a(et et2) {
        byte by2 = et2.readByte();
        this.a((by2 & 1) > 0);
        this.b((by2 & 2) > 0);
        this.c((by2 & 4) > 0);
        this.d((by2 & 8) > 0);
        this.a(et2.readFloat());
        this.b(et2.readFloat());
    }

    @Override
    public void b(et et2) {
        byte by2 = 0;
        if (this.c()) {
            by2 = (byte)(by2 | 1);
        }
        if (this.d()) {
            by2 = (byte)(by2 | 2);
        }
        if (this.e()) {
            by2 = (byte)(by2 | 4);
        }
        if (this.f()) {
            by2 = (byte)(by2 | 8);
        }
        et2.writeByte(by2);
        et2.writeFloat(this.e);
        et2.writeFloat(this.f);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("invuln=%b, flying=%b, canfly=%b, instabuild=%b, flyspeed=%.4f, walkspped=%.4f", this.c(), this.d(), this.e(), this.f(), Float.valueOf(this.g()), Float.valueOf(this.h()));
    }

    public boolean c() {
        return this.a;
    }

    public void a(boolean bl2) {
        this.a = bl2;
    }

    public boolean d() {
        return this.b;
    }

    public void b(boolean bl2) {
        this.b = bl2;
    }

    public boolean e() {
        return this.c;
    }

    public void c(boolean bl2) {
        this.c = bl2;
    }

    public boolean f() {
        return this.d;
    }

    public void d(boolean bl2) {
        this.d = bl2;
    }

    public float g() {
        return this.e;
    }

    public void a(float f2) {
        this.e = f2;
    }

    public float h() {
        return this.f;
    }

    public void b(float f2) {
        this.f = f2;
    }
}

