/*
 * Decompiled with CFR 0.152.
 */
public class uu
extends ui {
    private td a;
    private sv b;
    private double c;
    private double d;
    private double e;
    private double f;
    private float g;

    public uu(td td2, double d2, float f2) {
        this.a = td2;
        this.f = d2;
        this.g = f2;
        this.a(1);
    }

    @Override
    public boolean a() {
        this.b = this.a.o();
        if (this.b == null) {
            return false;
        }
        if (this.b.f(this.a) > (double)(this.g * this.g)) {
            return false;
        }
        azw azw2 = vx.a(this.a, 16, 7, azw.a(this.b.s, this.b.t, this.b.u));
        if (azw2 == null) {
            return false;
        }
        this.c = azw2.a;
        this.d = azw2.b;
        this.e = azw2.c;
        return true;
    }

    @Override
    public boolean b() {
        return !this.a.m().g() && this.b.Z() && this.b.f(this.a) < (double)(this.g * this.g);
    }

    @Override
    public void d() {
        this.b = null;
    }

    @Override
    public void c() {
        this.a.m().a(this.c, this.d, this.e, this.f);
    }
}

