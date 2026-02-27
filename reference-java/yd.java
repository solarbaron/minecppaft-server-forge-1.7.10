/*
 * Decompiled with CFR 0.152.
 */
public class yd
extends sr
implements yb {
    public int h;
    public double i;
    public double bm;
    public double bn;
    private sa bq;
    private int br;
    public int bo;
    public int bp;
    private int bs = 1;

    public yd(ahb ahb2) {
        super(ahb2);
        this.a(4.0f, 4.0f);
        this.ae = true;
        this.b = 5;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if ("fireball".equals(ro2.p()) && ro2.j() instanceof yz) {
            super.a(ro2, 1000.0f);
            ((yz)ro2.j()).a(pc.z);
            return true;
        }
        return super.a(ro2, f2);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(10.0);
    }

    @Override
    protected void bq() {
        byte by2;
        byte by3;
        if (!this.o.E && this.o.r == rd.a) {
            this.B();
        }
        this.w();
        this.bo = this.bp;
        double d2 = this.i - this.s;
        double d3 = this.bm - this.t;
        double d4 = this.bn - this.u;
        double d5 = d2 * d2 + d3 * d3 + d4 * d4;
        if (d5 < 1.0 || d5 > 3600.0) {
            this.i = this.s + (double)((this.Z.nextFloat() * 2.0f - 1.0f) * 16.0f);
            this.bm = this.t + (double)((this.Z.nextFloat() * 2.0f - 1.0f) * 16.0f);
            this.bn = this.u + (double)((this.Z.nextFloat() * 2.0f - 1.0f) * 16.0f);
        }
        if (this.h-- <= 0) {
            this.h += this.Z.nextInt(5) + 2;
            if (this.a(this.i, this.bm, this.bn, d5 = (double)qh.a(d5))) {
                this.v += d2 / d5 * 0.1;
                this.w += d3 / d5 * 0.1;
                this.x += d4 / d5 * 0.1;
            } else {
                this.i = this.s;
                this.bm = this.t;
                this.bn = this.u;
            }
        }
        if (this.bq != null && this.bq.K) {
            this.bq = null;
        }
        if (this.bq == null || this.br-- <= 0) {
            this.bq = this.o.b((sa)this, 100.0);
            if (this.bq != null) {
                this.br = 20;
            }
        }
        double d6 = 64.0;
        if (this.bq != null && this.bq.f(this) < d6 * d6) {
            double d7 = this.bq.s - this.s;
            double d8 = this.bq.C.b + (double)(this.bq.N / 2.0f) - (this.t + (double)(this.N / 2.0f));
            double d9 = this.bq.u - this.u;
            this.aM = this.y = -((float)Math.atan2(d7, d9)) * 180.0f / (float)Math.PI;
            if (this.p(this.bq)) {
                if (this.bp == 10) {
                    this.o.a(null, 1007, (int)this.s, (int)this.t, (int)this.u, 0);
                }
                ++this.bp;
                if (this.bp == 20) {
                    this.o.a(null, 1008, (int)this.s, (int)this.t, (int)this.u, 0);
                    zg zg2 = new zg(this.o, this, d7, d8, d9);
                    zg2.e = this.bs;
                    double d10 = 4.0;
                    azw azw2 = this.j(1.0f);
                    zg2.s = this.s + azw2.a * d10;
                    zg2.t = this.t + (double)(this.N / 2.0f) + 0.5;
                    zg2.u = this.u + azw2.c * d10;
                    this.o.d(zg2);
                    this.bp = -40;
                }
            } else if (this.bp > 0) {
                --this.bp;
            }
        } else {
            this.aM = this.y = -((float)Math.atan2(this.v, this.x)) * 180.0f / (float)Math.PI;
            if (this.bp > 0) {
                --this.bp;
            }
        }
        if (!this.o.E && (by3 = this.af.a(16)) != (by2 = (byte)(this.bp > 10 ? 1 : 0))) {
            this.af.b(16, by2);
        }
    }

    private boolean a(double d2, double d3, double d4, double d5) {
        double d6 = (this.i - this.s) / d5;
        double d7 = (this.bm - this.t) / d5;
        double d8 = (this.bn - this.u) / d5;
        azt azt2 = this.C.b();
        int n2 = 1;
        while ((double)n2 < d5) {
            azt2.d(d6, d7, d8);
            if (!this.o.a((sa)this, azt2).isEmpty()) {
                return false;
            }
            ++n2;
        }
        return true;
    }

    @Override
    protected String t() {
        return "mob.ghast.moan";
    }

    @Override
    protected String aT() {
        return "mob.ghast.scream";
    }

    @Override
    protected String aU() {
        return "mob.ghast.death";
    }

    @Override
    protected adb u() {
        return ade.H;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3;
        int n4 = this.Z.nextInt(2) + this.Z.nextInt(1 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.bk, 1);
        }
        n4 = this.Z.nextInt(3) + this.Z.nextInt(1 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.H, 1);
        }
    }

    @Override
    protected float bf() {
        return 10.0f;
    }

    @Override
    public boolean by() {
        return this.Z.nextInt(20) == 0 && super.by() && this.o.r != rd.a;
    }

    @Override
    public int bB() {
        return 1;
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("ExplosionPower", this.bs);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("ExplosionPower", 99)) {
            this.bs = dh2.f("ExplosionPower");
        }
    }
}

