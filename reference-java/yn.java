/*
 * Decompiled with CFR 0.152.
 */
public class yn
extends yg {
    public yn(ahb ahb2) {
        super(ahb2);
        this.a(1.4f, 0.9f);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
    }

    @Override
    public void h() {
        super.h();
        if (!this.o.E) {
            this.a(this.E);
        }
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(16.0);
        this.a(yj.d).a(0.8f);
    }

    @Override
    protected sa bR() {
        float f2 = this.d(1.0f);
        if (f2 < 0.5f) {
            double d2 = 16.0;
            return this.o.b((sa)this, d2);
        }
        return null;
    }

    @Override
    protected String t() {
        return "mob.spider.say";
    }

    @Override
    protected String aT() {
        return "mob.spider.say";
    }

    @Override
    protected String aU() {
        return "mob.spider.death";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.spider.step", 0.15f, 1.0f);
    }

    @Override
    protected void a(sa sa2, float f2) {
        float f3 = this.d(1.0f);
        if (f3 > 0.5f && this.Z.nextInt(100) == 0) {
            this.bm = null;
            return;
        }
        if (f2 > 2.0f && f2 < 6.0f && this.Z.nextInt(10) == 0) {
            if (this.D) {
                double d2 = sa2.s - this.s;
                double d3 = sa2.u - this.u;
                float f4 = qh.a(d2 * d2 + d3 * d3);
                this.v = d2 / (double)f4 * 0.5 * (double)0.8f + this.v * (double)0.2f;
                this.x = d3 / (double)f4 * 0.5 * (double)0.8f + this.x * (double)0.2f;
                this.w = 0.4f;
            }
        } else {
            super.a(sa2, f2);
        }
    }

    @Override
    protected adb u() {
        return ade.F;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        super.b(bl2, n2);
        if (bl2 && (this.Z.nextInt(3) == 0 || this.Z.nextInt(1 + n2) > 0)) {
            this.a(ade.bp, 1);
        }
    }

    @Override
    public boolean h_() {
        return this.bZ();
    }

    @Override
    public void as() {
    }

    @Override
    public sz bd() {
        return sz.c;
    }

    @Override
    public boolean d(rw rw2) {
        if (rw2.a() == rv.u.H) {
            return false;
        }
        return super.d(rw2);
    }

    public boolean bZ() {
        return (this.af.a(16) & 1) != 0;
    }

    public void a(boolean bl2) {
        byte by2 = this.af.a(16);
        by2 = bl2 ? (byte)(by2 | 1) : (byte)(by2 & 0xFFFFFFFE);
        this.af.b(16, by2);
    }

    @Override
    public sy a(sy sy2) {
        int n2;
        sy2 = super.a(sy2);
        if (this.o.s.nextInt(100) == 0) {
            yl yl2 = new yl(this.o);
            yl2.b(this.s, this.t, this.u, this.y, 0.0f);
            yl2.a((sy)null);
            this.o.d(yl2);
            yl2.a((sa)this);
        }
        if (sy2 == null) {
            sy2 = new yo();
            if (this.o.r == rd.d && this.o.s.nextFloat() < 0.1f * this.o.b(this.s, this.t, this.u)) {
                ((yo)sy2).a(this.o.s);
            }
        }
        if (sy2 instanceof yo && (n2 = ((yo)sy2).a) > 0 && rv.a[n2] != null) {
            this.c(new rw(n2, Integer.MAX_VALUE));
        }
        return sy2;
    }
}

