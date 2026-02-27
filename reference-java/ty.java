/*
 * Decompiled with CFR 0.152.
 */
public class ty
extends ui {
    private wv a;
    private yz b;
    private ahb c;
    private float d;
    private int e;

    public ty(wv wv2, float f2) {
        this.a = wv2;
        this.c = wv2.o;
        this.d = f2;
        this.a(2);
    }

    @Override
    public boolean a() {
        this.b = this.c.a((sa)this.a, this.d);
        if (this.b == null) {
            return false;
        }
        return this.a(this.b);
    }

    @Override
    public boolean b() {
        if (!this.b.Z()) {
            return false;
        }
        if (this.a.f((sa)this.b) > (double)(this.d * this.d)) {
            return false;
        }
        return this.e > 0 && this.a(this.b);
    }

    @Override
    public void c() {
        this.a.m(true);
        this.e = 40 + this.a.aI().nextInt(40);
    }

    @Override
    public void d() {
        this.a.m(false);
        this.b = null;
    }

    @Override
    public void e() {
        this.a.j().a(this.b.s, this.b.t + (double)this.b.g(), this.b.u, 10.0f, this.a.x());
        --this.e;
    }

    private boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 == null) {
            return false;
        }
        if (!this.a.bZ() && add2.b() == ade.aS) {
            return true;
        }
        return this.a.c(add2);
    }
}

