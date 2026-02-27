/*
 * Decompiled with CFR 0.152.
 */
public class uq
extends ui {
    ahb a;
    td b;
    int c;
    double d;
    boolean e;
    ayf f;
    Class g;
    private int h;
    private double i;
    private double j;
    private double k;

    public uq(td td2, Class clazz, double d2, boolean bl2) {
        this(td2, d2, bl2);
        this.g = clazz;
    }

    public uq(td td2, double d2, boolean bl2) {
        this.b = td2;
        this.a = td2.o;
        this.d = d2;
        this.e = bl2;
        this.a(3);
    }

    @Override
    public boolean a() {
        sv sv2 = this.b.o();
        if (sv2 == null) {
            return false;
        }
        if (!sv2.Z()) {
            return false;
        }
        if (this.g != null && !this.g.isAssignableFrom(sv2.getClass())) {
            return false;
        }
        this.f = this.b.m().a(sv2);
        return this.f != null;
    }

    @Override
    public boolean b() {
        sv sv2 = this.b.o();
        if (sv2 == null) {
            return false;
        }
        if (!sv2.Z()) {
            return false;
        }
        if (!this.e) {
            return !this.b.m().g();
        }
        return this.b.b(qh.c(sv2.s), qh.c(sv2.t), qh.c(sv2.u));
    }

    @Override
    public void c() {
        this.b.m().a(this.f, this.d);
        this.h = 0;
    }

    @Override
    public void d() {
        this.b.m().h();
    }

    @Override
    public void e() {
        sv sv2 = this.b.o();
        this.b.j().a(sv2, 30.0f, 30.0f);
        double d2 = this.b.e(sv2.s, sv2.C.b, sv2.u);
        double d3 = this.b.M * 2.0f * (this.b.M * 2.0f) + sv2.M;
        --this.h;
        if ((this.e || this.b.n().a(sv2)) && this.h <= 0 && (this.i == 0.0 && this.j == 0.0 && this.k == 0.0 || sv2.e(this.i, this.j, this.k) >= 1.0 || this.b.aI().nextFloat() < 0.05f)) {
            this.i = sv2.s;
            this.j = sv2.C.b;
            this.k = sv2.u;
            this.h = 4 + this.b.aI().nextInt(7);
            if (d2 > 1024.0) {
                this.h += 10;
            } else if (d2 > 256.0) {
                this.h += 5;
            }
            if (!this.b.m().a(sv2, this.d)) {
                this.h += 15;
            }
        }
        this.c = Math.max(this.c - 1, 0);
        if (d2 <= d3 && this.c <= 20) {
            this.c = 20;
            if (this.b.be() != null) {
                this.b.ba();
            }
            this.b.n(sv2);
        }
    }
}

