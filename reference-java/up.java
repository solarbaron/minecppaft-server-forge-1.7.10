/*
 * Decompiled with CFR 0.152.
 */
public class up
extends ui {
    private yv b;
    private yv c;
    private ahb d;
    private int e;
    vz a;

    public up(yv yv2) {
        this.b = yv2;
        this.d = yv2.o;
        this.a(3);
    }

    @Override
    public boolean a() {
        if (this.b.d() != 0) {
            return false;
        }
        if (this.b.aI().nextInt(500) != 0) {
            return false;
        }
        this.a = this.d.A.a(qh.c(this.b.s), qh.c(this.b.t), qh.c(this.b.u), 0);
        if (this.a == null) {
            return false;
        }
        if (!this.f()) {
            return false;
        }
        sa sa2 = this.d.a(yv.class, this.b.C.b(8.0, 3.0, 8.0), (sa)this.b);
        if (sa2 == null) {
            return false;
        }
        this.c = (yv)sa2;
        return this.c.d() == 0;
    }

    @Override
    public void c() {
        this.e = 300;
        this.b.i(true);
    }

    @Override
    public void d() {
        this.a = null;
        this.c = null;
        this.b.i(false);
    }

    @Override
    public boolean b() {
        return this.e >= 0 && this.f() && this.b.d() == 0;
    }

    @Override
    public void e() {
        --this.e;
        this.b.j().a(this.c, 10.0f, 30.0f);
        if (this.b.f(this.c) > 2.25) {
            this.b.m().a(this.c, 0.25);
        } else if (this.e == 0 && this.c.ca()) {
            this.g();
        }
        if (this.b.aI().nextInt(35) == 0) {
            this.d.a((sa)this.b, (byte)12);
        }
    }

    private boolean f() {
        if (!this.a.i()) {
            return false;
        }
        int n2 = (int)((double)this.a.c() * 0.35);
        return this.a.e() < n2;
    }

    private void g() {
        yv yv2 = this.b.b(this.c);
        this.c.c(6000);
        this.b.c(6000);
        yv2.c(-24000);
        yv2.b(this.b.s, this.b.t, this.b.u, 0.0f, 0.0f);
        this.d.d(yv2);
        this.d.a((sa)yv2, (byte)12);
    }
}

