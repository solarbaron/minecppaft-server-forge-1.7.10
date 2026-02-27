/*
 * Decompiled with CFR 0.152.
 */
public class xx
extends yg {
    private float bp = 0.5f;
    private int bq;
    private int br;

    public xx(ahb ahb2) {
        super(ahb2);
        this.ae = true;
        this.b = 10;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.e).a(6.0);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
    }

    @Override
    protected String t() {
        return "mob.blaze.breathe";
    }

    @Override
    protected String aT() {
        return "mob.blaze.hit";
    }

    @Override
    protected String aU() {
        return "mob.blaze.death";
    }

    @Override
    public float d(float f2) {
        return 1.0f;
    }

    @Override
    public void e() {
        if (!this.o.E) {
            if (this.L()) {
                this.a(ro.e, 1.0f);
            }
            --this.bq;
            if (this.bq <= 0) {
                this.bq = 100;
                this.bp = 0.5f + (float)this.Z.nextGaussian() * 3.0f;
            }
            if (this.bT() != null && this.bT().t + (double)this.bT().g() > this.t + (double)this.g() + (double)this.bp) {
                this.w += ((double)0.3f - this.w) * (double)0.3f;
            }
        }
        if (this.Z.nextInt(24) == 0) {
            this.o.a(this.s + 0.5, this.t + 0.5, this.u + 0.5, "fire.fire", 1.0f + this.Z.nextFloat(), this.Z.nextFloat() * 0.7f + 0.3f);
        }
        if (!this.D && this.w < 0.0) {
            this.w *= 0.6;
        }
        for (int i2 = 0; i2 < 2; ++i2) {
            this.o.a("largesmoke", this.s + (this.Z.nextDouble() - 0.5) * (double)this.M, this.t + this.Z.nextDouble() * (double)this.N, this.u + (this.Z.nextDouble() - 0.5) * (double)this.M, 0.0, 0.0, 0.0);
        }
        super.e();
    }

    @Override
    protected void a(sa sa2, float f2) {
        if (this.aB <= 0 && f2 < 2.0f && sa2.C.e > this.C.b && sa2.C.b < this.C.e) {
            this.aB = 20;
            this.n(sa2);
        } else if (f2 < 30.0f) {
            double d2 = sa2.s - this.s;
            double d3 = sa2.C.b + (double)(sa2.N / 2.0f) - (this.t + (double)(this.N / 2.0f));
            double d4 = sa2.u - this.u;
            if (this.aB == 0) {
                ++this.br;
                if (this.br == 1) {
                    this.aB = 60;
                    this.a(true);
                } else if (this.br <= 4) {
                    this.aB = 6;
                } else {
                    this.aB = 100;
                    this.br = 0;
                    this.a(false);
                }
                if (this.br > 1) {
                    float f3 = qh.c(f2) * 0.5f;
                    this.o.a(null, 1009, (int)this.s, (int)this.t, (int)this.u, 0);
                    for (int i2 = 0; i2 < 1; ++i2) {
                        zi zi2 = new zi(this.o, this, d2 + this.Z.nextGaussian() * (double)f3, d3, d4 + this.Z.nextGaussian() * (double)f3);
                        zi2.t = this.t + (double)(this.N / 2.0f) + 0.5;
                        this.o.d(zi2);
                    }
                }
            }
            this.y = (float)(Math.atan2(d4, d2) * 180.0 / 3.1415927410125732) - 90.0f;
            this.bn = true;
        }
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    protected adb u() {
        return ade.bj;
    }

    @Override
    public boolean al() {
        return this.bZ();
    }

    @Override
    protected void b(boolean bl2, int n2) {
        if (bl2) {
            int n3 = this.Z.nextInt(2 + n2);
            for (int i2 = 0; i2 < n3; ++i2) {
                this.a(ade.bj, 1);
            }
        }
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
    protected boolean j_() {
        return true;
    }
}

