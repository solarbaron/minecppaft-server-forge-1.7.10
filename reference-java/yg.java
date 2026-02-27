/*
 * Decompiled with CFR 0.152.
 */
public abstract class yg
extends td
implements yb {
    public yg(ahb ahb2) {
        super(ahb2);
        this.b = 5;
    }

    @Override
    public void e() {
        this.bb();
        float f2 = this.d(1.0f);
        if (f2 > 0.5f) {
            this.aU += 2;
        }
        super.e();
    }

    @Override
    public void h() {
        super.h();
        if (!this.o.E && this.o.r == rd.a) {
            this.B();
        }
    }

    @Override
    protected String H() {
        return "game.hostile.swim";
    }

    @Override
    protected String O() {
        return "game.hostile.swim.splash";
    }

    @Override
    protected sa bR() {
        yz yz2 = this.o.b((sa)this, 16.0);
        if (yz2 != null && this.p(yz2)) {
            return yz2;
        }
        return null;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (super.a(ro2, f2)) {
            sa sa2 = ro2.j();
            if (this.l == sa2 || this.m == sa2) {
                return true;
            }
            if (sa2 != this) {
                this.bm = sa2;
            }
            return true;
        }
        return false;
    }

    @Override
    protected String aT() {
        return "game.hostile.hurt";
    }

    @Override
    protected String aU() {
        return "game.hostile.die";
    }

    @Override
    protected String o(int n2) {
        if (n2 > 4) {
            return "game.hostile.hurt.fall.big";
        }
        return "game.hostile.hurt.fall.small";
    }

    @Override
    public boolean n(sa sa2) {
        boolean bl2;
        float f2 = (float)this.a(yj.e).e();
        int n2 = 0;
        if (sa2 instanceof sv) {
            f2 += afv.a((sv)this, (sv)sa2);
            n2 += afv.b((sv)this, (sv)sa2);
        }
        if (bl2 = sa2.a(ro.a(this), f2)) {
            int n3;
            if (n2 > 0) {
                sa2.g(-qh.a(this.y * (float)Math.PI / 180.0f) * (float)n2 * 0.5f, 0.1, qh.b(this.y * (float)Math.PI / 180.0f) * (float)n2 * 0.5f);
                this.v *= 0.6;
                this.x *= 0.6;
            }
            if ((n3 = afv.a(this)) > 0) {
                sa2.e(n3 * 4);
            }
            if (sa2 instanceof sv) {
                afv.a((sv)sa2, (sa)this);
            }
            afv.b((sv)this, sa2);
        }
        return bl2;
    }

    @Override
    protected void a(sa sa2, float f2) {
        if (this.aB <= 0 && f2 < 2.0f && sa2.C.e > this.C.b && sa2.C.b < this.C.e) {
            this.aB = 20;
            this.n(sa2);
        }
    }

    @Override
    public float a(int n2, int n3, int n4) {
        return 0.5f - this.o.n(n2, n3, n4);
    }

    protected boolean j_() {
        int n2;
        int n3;
        int n4 = qh.c(this.s);
        if (this.o.b(ahn.a, n4, n3 = qh.c(this.C.b), n2 = qh.c(this.u)) > this.Z.nextInt(32)) {
            return false;
        }
        int n5 = this.o.k(n4, n3, n2);
        if (this.o.P()) {
            int n6 = this.o.j;
            this.o.j = 10;
            n5 = this.o.k(n4, n3, n2);
            this.o.j = n6;
        }
        return n5 <= this.Z.nextInt(8);
    }

    @Override
    public boolean by() {
        return this.o.r != rd.a && this.j_() && super.by();
    }

    @Override
    protected void aD() {
        super.aD();
        this.bc().b(yj.e);
    }

    @Override
    protected boolean aG() {
        return true;
    }
}

