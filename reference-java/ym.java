/*
 * Decompiled with CFR 0.152.
 */
public class ym
extends sw
implements yb {
    public float h;
    public float i;
    public float bm;
    private int bn;

    public ym(ahb ahb2) {
        super(ahb2);
        int n2 = 1 << this.Z.nextInt(3);
        this.L = 0.0f;
        this.bn = this.Z.nextInt(20) + 10;
        this.a(n2);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(1));
    }

    protected void a(int n2) {
        this.af.b(16, new Byte((byte)n2));
        this.a(0.6f * (float)n2, 0.6f * (float)n2);
        this.b(this.s, this.t, this.u);
        this.a(yj.a).a(n2 * n2);
        this.g(this.aY());
        this.b = n2;
    }

    public int bX() {
        return this.af.a(16);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Size", this.bX() - 1);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        int n2 = dh2.f("Size");
        if (n2 < 0) {
            n2 = 0;
        }
        this.a(n2 + 1);
    }

    protected String bP() {
        return "slime";
    }

    protected String bV() {
        return "mob.slime." + (this.bX() > 1 ? "big" : "small");
    }

    @Override
    public void h() {
        int n2;
        if (!this.o.E && this.o.r == rd.a && this.bX() > 0) {
            this.K = true;
        }
        this.i += (this.h - this.i) * 0.5f;
        this.bm = this.i;
        boolean bl2 = this.D;
        super.h();
        if (this.D && !bl2) {
            n2 = this.bX();
            for (int i2 = 0; i2 < n2 * 8; ++i2) {
                float f2 = this.Z.nextFloat() * (float)Math.PI * 2.0f;
                float f3 = this.Z.nextFloat() * 0.5f + 0.5f;
                float f4 = qh.a(f2) * (float)n2 * 0.5f * f3;
                float f5 = qh.b(f2) * (float)n2 * 0.5f * f3;
                this.o.a(this.bP(), this.s + (double)f4, this.C.b, this.u + (double)f5, 0.0, 0.0, 0.0);
            }
            if (this.bW()) {
                this.a(this.bV(), this.bf(), ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f) / 0.8f);
            }
            this.h = -0.5f;
        } else if (!this.D && bl2) {
            this.h = 1.0f;
        }
        this.bS();
        if (this.o.E) {
            n2 = this.bX();
            this.a(0.6f * (float)n2, 0.6f * (float)n2);
        }
    }

    @Override
    protected void bq() {
        this.w();
        yz yz2 = this.o.b((sa)this, 16.0);
        if (yz2 != null) {
            this.a(yz2, 10.0f, 20.0f);
        }
        if (this.D && this.bn-- <= 0) {
            this.bn = this.bR();
            if (yz2 != null) {
                this.bn /= 3;
            }
            this.bc = true;
            if (this.bY()) {
                this.a(this.bV(), this.bf(), ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f) * 0.8f);
            }
            this.bd = 1.0f - this.Z.nextFloat() * 2.0f;
            this.be = 1 * this.bX();
        } else {
            this.bc = false;
            if (this.D) {
                this.be = 0.0f;
                this.bd = 0.0f;
            }
        }
    }

    protected void bS() {
        this.h *= 0.6f;
    }

    protected int bR() {
        return this.Z.nextInt(20) + 10;
    }

    protected ym bQ() {
        return new ym(this.o);
    }

    @Override
    public void B() {
        int n2 = this.bX();
        if (!this.o.E && n2 > 1 && this.aS() <= 0.0f) {
            int n3 = 2 + this.Z.nextInt(3);
            for (int i2 = 0; i2 < n3; ++i2) {
                float f2 = ((float)(i2 % 2) - 0.5f) * (float)n2 / 4.0f;
                float f3 = ((float)(i2 / 2) - 0.5f) * (float)n2 / 4.0f;
                ym ym2 = this.bQ();
                ym2.a(n2 / 2);
                ym2.b(this.s + (double)f2, this.t + 0.5, this.u + (double)f3, this.Z.nextFloat() * 360.0f, 0.0f);
                this.o.d(ym2);
            }
        }
        super.B();
    }

    @Override
    public void b_(yz yz2) {
        if (this.bT()) {
            int n2 = this.bX();
            if (this.p(yz2) && this.f(yz2) < 0.6 * (double)n2 * (0.6 * (double)n2) && yz2.a(ro.a(this), (float)this.bU())) {
                this.a("mob.attack", 1.0f, (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f);
            }
        }
    }

    protected boolean bT() {
        return this.bX() > 1;
    }

    protected int bU() {
        return this.bX();
    }

    @Override
    protected String aT() {
        return "mob.slime." + (this.bX() > 1 ? "big" : "small");
    }

    @Override
    protected String aU() {
        return "mob.slime." + (this.bX() > 1 ? "big" : "small");
    }

    @Override
    protected adb u() {
        if (this.bX() == 1) {
            return ade.aH;
        }
        return adb.d(0);
    }

    @Override
    public boolean by() {
        apx apx2 = this.o.d(qh.c(this.s), qh.c(this.u));
        if (this.o.N().u() == ahm.c && this.Z.nextInt(4) != 1) {
            return false;
        }
        if (this.bX() == 1 || this.o.r != rd.a) {
            ahu ahu2 = this.o.a(qh.c(this.s), qh.c(this.u));
            if (ahu2 == ahu.u && this.t > 50.0 && this.t < 70.0 && this.Z.nextFloat() < 0.5f && this.Z.nextFloat() < this.o.y() && this.o.k(qh.c(this.s), qh.c(this.t), qh.c(this.u)) <= this.Z.nextInt(8)) {
                return super.by();
            }
            if (this.Z.nextInt(10) == 0 && apx2.a(987234911L).nextInt(10) == 0 && this.t < 40.0) {
                return super.by();
            }
        }
        return false;
    }

    @Override
    protected float bf() {
        return 0.4f * (float)this.bX();
    }

    @Override
    public int x() {
        return 0;
    }

    protected boolean bY() {
        return this.bX() > 0;
    }

    protected boolean bW() {
        return this.bX() > 2;
    }
}

