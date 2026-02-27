/*
 * Decompiled with CFR 0.152.
 */
public class xc
extends yg
implements yi {
    private float[] bp = new float[2];
    private float[] bq = new float[2];
    private float[] br = new float[2];
    private float[] bs = new float[2];
    private int[] bt = new int[2];
    private int[] bu = new int[2];
    private int bv;
    private static final sj bw = new xd();

    public xc(ahb ahb2) {
        super(ahb2);
        this.g(this.aY());
        this.a(0.9f, 4.0f);
        this.ae = true;
        this.m().e(true);
        this.c.a(0, new uf(this));
        this.c.a(2, new vd(this, 1.0, 40, 20.0f));
        this.c.a(5, new vc(this, 1.0));
        this.c.a(6, new un(this, yz.class, 8.0f));
        this.c.a(7, new vb(this));
        this.d.a(1, new vn(this, false));
        this.d.a(2, new vo(this, sw.class, 0, false, false, bw));
        this.b = 50;
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(17, new Integer(0));
        this.af.a(18, new Integer(0));
        this.af.a(19, new Integer(0));
        this.af.a(20, new Integer(0));
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Invul", this.ca());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.s(dh2.f("Invul"));
    }

    @Override
    protected String t() {
        return "mob.wither.idle";
    }

    @Override
    protected String aT() {
        return "mob.wither.hurt";
    }

    @Override
    protected String aU() {
        return "mob.wither.death";
    }

    @Override
    public void e() {
        int n2;
        int n3;
        double d2;
        double d3;
        double d4;
        sa sa2;
        this.w *= (double)0.6f;
        if (!this.o.E && this.t(0) > 0 && (sa2 = this.o.a(this.t(0))) != null) {
            double d5;
            if (this.t < sa2.t || !this.cb() && this.t < sa2.t + 5.0) {
                if (this.w < 0.0) {
                    this.w = 0.0;
                }
                this.w += (0.5 - this.w) * (double)0.6f;
            }
            if ((d4 = (d5 = sa2.s - this.s) * d5 + (d3 = sa2.u - this.u) * d3) > 9.0) {
                d2 = qh.a(d4);
                this.v += (d5 / d2 * 0.5 - this.v) * (double)0.6f;
                this.x += (d3 / d2 * 0.5 - this.x) * (double)0.6f;
            }
        }
        if (this.v * this.v + this.x * this.x > (double)0.05f) {
            this.y = (float)Math.atan2(this.x, this.v) * 57.295776f - 90.0f;
        }
        super.e();
        for (n3 = 0; n3 < 2; ++n3) {
            this.bs[n3] = this.bq[n3];
            this.br[n3] = this.bp[n3];
        }
        for (n3 = 0; n3 < 2; ++n3) {
            int n4 = this.t(n3 + 1);
            sa sa3 = null;
            if (n4 > 0) {
                sa3 = this.o.a(n4);
            }
            if (sa3 != null) {
                d3 = this.u(n3 + 1);
                d4 = this.v(n3 + 1);
                d2 = this.w(n3 + 1);
                double d6 = sa3.s - d3;
                double d7 = sa3.t + (double)sa3.g() - d4;
                double d8 = sa3.u - d2;
                double d9 = qh.a(d6 * d6 + d8 * d8);
                float f2 = (float)(Math.atan2(d8, d6) * 180.0 / 3.1415927410125732) - 90.0f;
                float f3 = (float)(-(Math.atan2(d7, d9) * 180.0 / 3.1415927410125732));
                this.bp[n3] = this.b(this.bp[n3], f3, 40.0f);
                this.bq[n3] = this.b(this.bq[n3], f2, 10.0f);
                continue;
            }
            this.bq[n3] = this.b(this.bq[n3], this.aM, 10.0f);
        }
        n3 = this.cb() ? 1 : 0;
        for (n2 = 0; n2 < 3; ++n2) {
            double d10 = this.u(n2);
            double d11 = this.v(n2);
            double d12 = this.w(n2);
            this.o.a("smoke", d10 + this.Z.nextGaussian() * (double)0.3f, d11 + this.Z.nextGaussian() * (double)0.3f, d12 + this.Z.nextGaussian() * (double)0.3f, 0.0, 0.0, 0.0);
            if (n3 == 0 || this.o.s.nextInt(4) != 0) continue;
            this.o.a("mobSpell", d10 + this.Z.nextGaussian() * (double)0.3f, d11 + this.Z.nextGaussian() * (double)0.3f, d12 + this.Z.nextGaussian() * (double)0.3f, (double)0.7f, (double)0.7f, 0.5);
        }
        if (this.ca() > 0) {
            for (n2 = 0; n2 < 3; ++n2) {
                this.o.a("mobSpell", this.s + this.Z.nextGaussian() * 1.0, this.t + (double)(this.Z.nextFloat() * 3.3f), this.u + this.Z.nextGaussian() * 1.0, (double)0.7f, (double)0.7f, 0.9f);
            }
        }
    }

    @Override
    protected void bn() {
        int n2;
        int n3;
        if (this.ca() > 0) {
            int n4 = this.ca() - 1;
            if (n4 <= 0) {
                this.o.a((sa)this, this.s, this.t + (double)this.g(), this.u, 7.0f, false, this.o.O().b("mobGriefing"));
                this.o.b(1013, (int)this.s, (int)this.t, (int)this.u, 0);
            }
            this.s(n4);
            if (this.aa % 10 == 0) {
                this.f(10.0f);
            }
            return;
        }
        super.bn();
        block0: for (n3 = 1; n3 < 3; ++n3) {
            Object object;
            if (this.aa < this.bt[n3 - 1]) continue;
            this.bt[n3 - 1] = this.aa + 10 + this.Z.nextInt(10);
            if (this.o.r == rd.c || this.o.r == rd.d) {
                int n5 = n3 - 1;
                int n6 = this.bu[n5];
                this.bu[n5] = n6 + 1;
                if (n6 > 15) {
                    float f2 = 10.0f;
                    float f3 = 5.0f;
                    double d2 = qh.a(this.Z, this.s - (double)f2, this.s + (double)f2);
                    double d3 = qh.a(this.Z, this.t - (double)f3, this.t + (double)f3);
                    double d4 = qh.a(this.Z, this.u - (double)f2, this.u + (double)f2);
                    this.a(n3 + 1, d2, d3, d4, true);
                    this.bu[n3 - 1] = 0;
                }
            }
            if ((n2 = this.t(n3)) > 0) {
                object = this.o.a(n2);
                if (object == null || !((sa)object).Z() || this.f((sa)object) > 900.0 || !this.p((sa)object)) {
                    this.b(n3, 0);
                    continue;
                }
                this.a(n3 + 1, (sv)object);
                this.bt[n3 - 1] = this.aa + 40 + this.Z.nextInt(20);
                this.bu[n3 - 1] = 0;
                continue;
            }
            object = this.o.a(sv.class, this.C.b(20.0, 8.0, 20.0), bw);
            for (int i2 = 0; i2 < 10 && !object.isEmpty(); ++i2) {
                sv sv2 = (sv)object.get(this.Z.nextInt(object.size()));
                if (sv2 != this && sv2.Z() && this.p(sv2)) {
                    if (sv2 instanceof yz) {
                        if (((yz)sv2).bE.a) continue block0;
                        this.b(n3, sv2.y());
                        continue block0;
                    }
                    this.b(n3, sv2.y());
                    continue block0;
                }
                object.remove(sv2);
            }
        }
        if (this.o() != null) {
            this.b(0, this.o().y());
        } else {
            this.b(0, 0);
        }
        if (this.bv > 0) {
            --this.bv;
            if (this.bv == 0 && this.o.O().b("mobGriefing")) {
                n3 = qh.c(this.t);
                n2 = qh.c(this.s);
                int n7 = qh.c(this.u);
                boolean bl2 = false;
                for (int i3 = -1; i3 <= 1; ++i3) {
                    for (int i4 = -1; i4 <= 1; ++i4) {
                        for (int i5 = 0; i5 <= 3; ++i5) {
                            int n8 = n2 + i3;
                            int n9 = n3 + i5;
                            int n10 = n7 + i4;
                            aji aji2 = this.o.a(n8, n9, n10);
                            if (aji2.o() == awt.a || aji2 == ajn.h || aji2 == ajn.bq || aji2 == ajn.br || aji2 == ajn.bI) continue;
                            bl2 = this.o.a(n8, n9, n10, true) || bl2;
                        }
                    }
                }
                if (bl2) {
                    this.o.a(null, 1012, (int)this.s, (int)this.t, (int)this.u, 0);
                }
            }
        }
        if (this.aa % 20 == 0) {
            this.f(1.0f);
        }
    }

    public void bZ() {
        this.s(220);
        this.g(this.aY() / 3.0f);
    }

    @Override
    public void as() {
    }

    @Override
    public int aV() {
        return 4;
    }

    private double u(int n2) {
        if (n2 <= 0) {
            return this.s;
        }
        float f2 = (this.aM + (float)(180 * (n2 - 1))) / 180.0f * (float)Math.PI;
        float f3 = qh.b(f2);
        return this.s + (double)f3 * 1.3;
    }

    private double v(int n2) {
        if (n2 <= 0) {
            return this.t + 3.0;
        }
        return this.t + 2.2;
    }

    private double w(int n2) {
        if (n2 <= 0) {
            return this.u;
        }
        float f2 = (this.aM + (float)(180 * (n2 - 1))) / 180.0f * (float)Math.PI;
        float f3 = qh.a(f2);
        return this.u + (double)f3 * 1.3;
    }

    private float b(float f2, float f3, float f4) {
        float f5 = qh.g(f3 - f2);
        if (f5 > f4) {
            f5 = f4;
        }
        if (f5 < -f4) {
            f5 = -f4;
        }
        return f2 + f5;
    }

    private void a(int n2, sv sv2) {
        this.a(n2, sv2.s, sv2.t + (double)sv2.g() * 0.5, sv2.u, n2 == 0 && this.Z.nextFloat() < 0.001f);
    }

    private void a(int n2, double d2, double d3, double d4, boolean bl2) {
        this.o.a(null, 1014, (int)this.s, (int)this.t, (int)this.u, 0);
        double d5 = this.u(n2);
        double d6 = this.v(n2);
        double d7 = this.w(n2);
        double d8 = d2 - d5;
        double d9 = d3 - d6;
        double d10 = d4 - d7;
        zp zp2 = new zp(this.o, this, d8, d9, d10);
        if (bl2) {
            zp2.a(true);
        }
        zp2.t = d6;
        zp2.s = d5;
        zp2.u = d7;
        this.o.d(zp2);
    }

    @Override
    public void a(sv sv2, float f2) {
        this.a(0, sv2);
    }

    @Override
    public boolean a(ro ro2, float f2) {
        sa sa2;
        if (this.aw()) {
            return false;
        }
        if (ro2 == ro.e) {
            return false;
        }
        if (this.ca() > 0) {
            return false;
        }
        if (this.cb() && (sa2 = ro2.i()) instanceof zc) {
            return false;
        }
        sa2 = ro2.j();
        if (sa2 != null && !(sa2 instanceof yz) && sa2 instanceof sv && ((sv)sa2).bd() == this.bd()) {
            return false;
        }
        if (this.bv <= 0) {
            this.bv = 20;
        }
        int n2 = 0;
        while (n2 < this.bu.length) {
            int n3 = n2++;
            this.bu[n3] = this.bu[n3] + 3;
        }
        return super.a(ro2, f2);
    }

    @Override
    protected void b(boolean bl2, int n2) {
        this.a(ade.bN, 1);
        if (!this.o.E) {
            for (yz yz2 : this.o.a(yz.class, this.C.b(50.0, 100.0, 50.0))) {
                yz2.a(pc.J);
            }
        }
    }

    @Override
    protected void w() {
        this.aU = 0;
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    public void c(rw rw2) {
    }

    @Override
    protected boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(300.0);
        this.a(yj.d).a(0.6f);
        this.a(yj.b).a(40.0);
    }

    public int ca() {
        return this.af.c(20);
    }

    public void s(int n2) {
        this.af.b(20, n2);
    }

    public int t(int n2) {
        return this.af.c(17 + n2);
    }

    public void b(int n2, int n3) {
        this.af.b(17 + n2, n3);
    }

    public boolean cb() {
        return this.aS() <= this.aY() / 2.0f;
    }

    @Override
    public sz bd() {
        return sz.b;
    }

    @Override
    public void a(sa sa2) {
        this.m = null;
    }
}

