/*
 * Decompiled with CFR 0.152.
 */
public class zp
extends ze {
    public zp(ahb ahb2) {
        super(ahb2);
        this.a(0.3125f, 0.3125f);
    }

    public zp(ahb ahb2, sv sv2, double d2, double d3, double d4) {
        super(ahb2, sv2, d2, d3, d4);
        this.a(0.3125f, 0.3125f);
    }

    @Override
    protected float e() {
        return this.f() ? 0.73f : super.e();
    }

    @Override
    public boolean al() {
        return false;
    }

    @Override
    public float a(agw agw2, ahb ahb2, int n2, int n3, int n4, aji aji2) {
        float f2 = super.a(agw2, ahb2, n2, n3, n4, aji2);
        if (this.f() && aji2 != ajn.h && aji2 != ajn.bq && aji2 != ajn.br && aji2 != ajn.bI) {
            f2 = Math.min(0.8f, f2);
        }
        return f2;
    }

    @Override
    protected void a(azu azu2) {
        if (!this.o.E) {
            if (azu2.g != null) {
                if (this.a != null) {
                    if (azu2.g.a(ro.a(this.a), 8.0f) && !azu2.g.Z()) {
                        this.a.f(5.0f);
                    }
                } else {
                    azu2.g.a(ro.k, 5.0f);
                }
                if (azu2.g instanceof sv) {
                    int n2 = 0;
                    if (this.o.r == rd.c) {
                        n2 = 10;
                    } else if (this.o.r == rd.d) {
                        n2 = 40;
                    }
                    if (n2 > 0) {
                        ((sv)azu2.g).c(new rw(rv.v.H, 20 * n2, 1));
                    }
                }
            }
            this.o.a((sa)this, this.s, this.t, this.u, 1.0f, false, this.o.O().b("mobGriefing"));
            this.B();
        }
    }

    @Override
    public boolean R() {
        return false;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        return false;
    }

    @Override
    protected void c() {
        this.af.a(10, (Object)0);
    }

    public boolean f() {
        return this.af.a(10) == 1;
    }

    public void a(boolean bl2) {
        this.af.b(10, bl2 ? (byte)1 : 0);
    }
}

