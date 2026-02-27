/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class xa
extends sw
implements wx,
yb {
    public double h;
    public double i;
    public double bm;
    public double[][] bn = new double[64][3];
    public int bo = -1;
    public wy[] bp;
    public wy bq = new wy(this, "head", 6.0f, 6.0f);
    public wy br = new wy(this, "body", 8.0f, 8.0f);
    public wy bs = new wy(this, "tail", 4.0f, 4.0f);
    public wy bt = new wy(this, "tail", 4.0f, 4.0f);
    public wy bu = new wy(this, "tail", 4.0f, 4.0f);
    public wy bv = new wy(this, "wing", 4.0f, 4.0f);
    public wy bw = new wy(this, "wing", 4.0f, 4.0f);
    public float bx;
    public float by;
    public boolean bz;
    public boolean bA;
    private sa bD;
    public int bB;
    public wz bC;

    public xa(ahb ahb2) {
        super(ahb2);
        this.bp = new wy[]{this.bq, this.br, this.bs, this.bt, this.bu, this.bv, this.bw};
        this.g(this.aY());
        this.a(16.0f, 8.0f);
        this.X = true;
        this.ae = true;
        this.i = 100.0;
        this.ak = true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(200.0);
    }

    @Override
    protected void c() {
        super.c();
    }

    public double[] b(int n2, float f2) {
        if (this.aS() <= 0.0f) {
            f2 = 0.0f;
        }
        f2 = 1.0f - f2;
        int n3 = this.bo - n2 * 1 & 0x3F;
        int n4 = this.bo - n2 * 1 - 1 & 0x3F;
        double[] dArray = new double[3];
        double d2 = this.bn[n3][0];
        double d3 = qh.g(this.bn[n4][0] - d2);
        dArray[0] = d2 + d3 * (double)f2;
        d2 = this.bn[n3][1];
        d3 = this.bn[n4][1] - d2;
        dArray[1] = d2 + d3 * (double)f2;
        dArray[2] = this.bn[n3][2] + (this.bn[n4][2] - this.bn[n3][2]) * (double)f2;
        return dArray;
    }

    @Override
    public void e() {
        float f2;
        double d2;
        double d3;
        double d4;
        float f3;
        float f4;
        if (this.o.E) {
            f4 = qh.b(this.by * (float)Math.PI * 2.0f);
            f3 = qh.b(this.bx * (float)Math.PI * 2.0f);
            if (f3 <= -0.3f && f4 >= -0.3f) {
                this.o.a(this.s, this.t, this.u, "mob.enderdragon.wings", 5.0f, 0.8f + this.Z.nextFloat() * 0.3f, false);
            }
        }
        this.bx = this.by;
        if (this.aS() <= 0.0f) {
            f4 = (this.Z.nextFloat() - 0.5f) * 8.0f;
            f3 = (this.Z.nextFloat() - 0.5f) * 4.0f;
            float f5 = (this.Z.nextFloat() - 0.5f) * 8.0f;
            this.o.a("largeexplode", this.s + (double)f4, this.t + 2.0 + (double)f3, this.u + (double)f5, 0.0, 0.0, 0.0);
            return;
        }
        this.bP();
        f4 = 0.2f / (qh.a(this.v * this.v + this.x * this.x) * 10.0f + 1.0f);
        this.by = this.bA ? (this.by += f4 * 0.5f) : (this.by += (f4 *= (float)Math.pow(2.0, this.w)));
        this.y = qh.g(this.y);
        if (this.bo < 0) {
            for (int i2 = 0; i2 < this.bn.length; ++i2) {
                this.bn[i2][0] = this.y;
                this.bn[i2][1] = this.t;
            }
        }
        if (++this.bo == this.bn.length) {
            this.bo = 0;
        }
        this.bn[this.bo][0] = this.y;
        this.bn[this.bo][1] = this.t;
        if (this.o.E) {
            if (this.bg > 0) {
                double d5 = this.s + (this.bh - this.s) / (double)this.bg;
                d4 = this.t + (this.bi - this.t) / (double)this.bg;
                d3 = this.u + (this.bj - this.u) / (double)this.bg;
                d2 = qh.g(this.bk - (double)this.y);
                this.y = (float)((double)this.y + d2 / (double)this.bg);
                this.z = (float)((double)this.z + (this.bl - (double)this.z) / (double)this.bg);
                --this.bg;
                this.b(d5, d4, d3);
                this.b(this.y, this.z);
            }
        } else {
            double d6 = this.h - this.s;
            d4 = this.i - this.t;
            d3 = this.bm - this.u;
            d2 = d6 * d6 + d4 * d4 + d3 * d3;
            if (this.bD != null) {
                this.h = this.bD.s;
                this.bm = this.bD.u;
                double d7 = this.h - this.s;
                double d8 = this.bm - this.u;
                double d9 = Math.sqrt(d7 * d7 + d8 * d8);
                double d10 = (double)0.4f + d9 / 80.0 - 1.0;
                if (d10 > 10.0) {
                    d10 = 10.0;
                }
                this.i = this.bD.C.b + d10;
            } else {
                this.h += this.Z.nextGaussian() * 2.0;
                this.bm += this.Z.nextGaussian() * 2.0;
            }
            if (this.bz || d2 < 100.0 || d2 > 22500.0 || this.E || this.F) {
                this.bQ();
            }
            if ((d4 /= (double)qh.a(d6 * d6 + d3 * d3)) < (double)(-(f2 = 0.6f))) {
                d4 = -f2;
            }
            if (d4 > (double)f2) {
                d4 = f2;
            }
            this.w += d4 * (double)0.1f;
            this.y = qh.g(this.y);
            double d11 = 180.0 - Math.atan2(d6, d3) * 180.0 / 3.1415927410125732;
            double d12 = qh.g(d11 - (double)this.y);
            if (d12 > 50.0) {
                d12 = 50.0;
            }
            if (d12 < -50.0) {
                d12 = -50.0;
            }
            azw azw2 = azw.a(this.h - this.s, this.i - this.t, this.bm - this.u).a();
            azw azw3 = azw.a(qh.a(this.y * (float)Math.PI / 180.0f), this.w, -qh.b(this.y * (float)Math.PI / 180.0f)).a();
            float f6 = (float)(azw3.b(azw2) + 0.5) / 1.5f;
            if (f6 < 0.0f) {
                f6 = 0.0f;
            }
            this.bf *= 0.8f;
            float f7 = qh.a(this.v * this.v + this.x * this.x) * 1.0f + 1.0f;
            double d13 = Math.sqrt(this.v * this.v + this.x * this.x) * 1.0 + 1.0;
            if (d13 > 40.0) {
                d13 = 40.0;
            }
            this.bf = (float)((double)this.bf + d12 * ((double)0.7f / d13 / (double)f7));
            this.y += this.bf * 0.1f;
            float f8 = (float)(2.0 / (d13 + 1.0));
            float f9 = 0.06f;
            this.a(0.0f, -1.0f, f9 * (f6 * f8 + (1.0f - f8)));
            if (this.bA) {
                this.d(this.v * (double)0.8f, this.w * (double)0.8f, this.x * (double)0.8f);
            } else {
                this.d(this.v, this.w, this.x);
            }
            azw azw4 = azw.a(this.v, this.w, this.x).a();
            float f10 = (float)(azw4.b(azw3) + 1.0) / 2.0f;
            f10 = 0.8f + 0.15f * f10;
            this.v *= (double)f10;
            this.x *= (double)f10;
            this.w *= (double)0.91f;
        }
        this.aM = this.y;
        this.bq.N = 3.0f;
        this.bq.M = 3.0f;
        this.bs.N = 2.0f;
        this.bs.M = 2.0f;
        this.bt.N = 2.0f;
        this.bt.M = 2.0f;
        this.bu.N = 2.0f;
        this.bu.M = 2.0f;
        this.br.N = 3.0f;
        this.br.M = 5.0f;
        this.bv.N = 2.0f;
        this.bv.M = 4.0f;
        this.bw.N = 3.0f;
        this.bw.M = 4.0f;
        f3 = (float)(this.b(5, 1.0f)[1] - this.b(10, 1.0f)[1]) * 10.0f / 180.0f * (float)Math.PI;
        float f11 = qh.b(f3);
        float f12 = -qh.a(f3);
        float f13 = this.y * (float)Math.PI / 180.0f;
        float f14 = qh.a(f13);
        float f15 = qh.b(f13);
        this.br.h();
        this.br.b(this.s + (double)(f14 * 0.5f), this.t, this.u - (double)(f15 * 0.5f), 0.0f, 0.0f);
        this.bv.h();
        this.bv.b(this.s + (double)(f15 * 4.5f), this.t + 2.0, this.u + (double)(f14 * 4.5f), 0.0f, 0.0f);
        this.bw.h();
        this.bw.b(this.s - (double)(f15 * 4.5f), this.t + 2.0, this.u - (double)(f14 * 4.5f), 0.0f, 0.0f);
        if (!this.o.E && this.ax == 0) {
            this.a(this.o.b((sa)this, this.bv.C.b(4.0, 2.0, 4.0).d(0.0, -2.0, 0.0)));
            this.a(this.o.b((sa)this, this.bw.C.b(4.0, 2.0, 4.0).d(0.0, -2.0, 0.0)));
            this.b(this.o.b((sa)this, this.bq.C.b(1.0, 1.0, 1.0)));
        }
        double[] dArray = this.b(5, 1.0f);
        double[] dArray2 = this.b(0, 1.0f);
        f2 = qh.a(this.y * (float)Math.PI / 180.0f - this.bf * 0.01f);
        float f16 = qh.b(this.y * (float)Math.PI / 180.0f - this.bf * 0.01f);
        this.bq.h();
        this.bq.b(this.s + (double)(f2 * 5.5f * f11), this.t + (dArray2[1] - dArray[1]) * 1.0 + (double)(f12 * 5.5f), this.u - (double)(f16 * 5.5f * f11), 0.0f, 0.0f);
        for (int i3 = 0; i3 < 3; ++i3) {
            wy wy2 = null;
            if (i3 == 0) {
                wy2 = this.bs;
            }
            if (i3 == 1) {
                wy2 = this.bt;
            }
            if (i3 == 2) {
                wy2 = this.bu;
            }
            double[] dArray3 = this.b(12 + i3 * 2, 1.0f);
            float f17 = this.y * (float)Math.PI / 180.0f + this.b(dArray3[0] - dArray[0]) * (float)Math.PI / 180.0f * 1.0f;
            float f18 = qh.a(f17);
            float f19 = qh.b(f17);
            float f20 = 1.5f;
            float f21 = (float)(i3 + 1) * 2.0f;
            wy2.h();
            wy2.b(this.s - (double)((f14 * f20 + f18 * f21) * f11), this.t + (dArray3[1] - dArray[1]) * 1.0 - (double)((f21 + f20) * f12) + 1.5, this.u + (double)((f15 * f20 + f19 * f21) * f11), 0.0f, 0.0f);
        }
        if (!this.o.E) {
            this.bA = this.a(this.bq.C) | this.a(this.br.C);
        }
    }

    private void bP() {
        if (this.bC != null) {
            if (this.bC.K) {
                if (!this.o.E) {
                    this.a(this.bq, ro.a(null), 10.0f);
                }
                this.bC = null;
            } else if (this.aa % 10 == 0 && this.aS() < this.aY()) {
                this.g(this.aS() + 1.0f);
            }
        }
        if (this.Z.nextInt(10) == 0) {
            float f2 = 32.0f;
            List list = this.o.a(wz.class, this.C.b(f2, f2, f2));
            wz wz2 = null;
            double d2 = Double.MAX_VALUE;
            for (wz wz3 : list) {
                double d3 = wz3.f(this);
                if (!(d3 < d2)) continue;
                d2 = d3;
                wz2 = wz3;
            }
            this.bC = wz2;
        }
    }

    private void a(List list) {
        double d2 = (this.br.C.a + this.br.C.d) / 2.0;
        double d3 = (this.br.C.c + this.br.C.f) / 2.0;
        for (sa sa2 : list) {
            if (!(sa2 instanceof sv)) continue;
            double d4 = sa2.s - d2;
            double d5 = sa2.u - d3;
            double d6 = d4 * d4 + d5 * d5;
            sa2.g(d4 / d6 * 4.0, 0.2f, d5 / d6 * 4.0);
        }
    }

    private void b(List list) {
        for (int i2 = 0; i2 < list.size(); ++i2) {
            sa sa2 = (sa)list.get(i2);
            if (!(sa2 instanceof sv)) continue;
            sa2.a(ro.a(this), 10.0f);
        }
    }

    private void bQ() {
        this.bz = false;
        if (this.Z.nextInt(2) == 0 && !this.o.h.isEmpty()) {
            this.bD = (sa)this.o.h.get(this.Z.nextInt(this.o.h.size()));
        } else {
            double d2;
            double d3;
            double d4;
            boolean bl2 = false;
            do {
                this.h = 0.0;
                this.i = 70.0f + this.Z.nextFloat() * 50.0f;
                this.bm = 0.0;
                this.h += (double)(this.Z.nextFloat() * 120.0f - 60.0f);
                this.bm += (double)(this.Z.nextFloat() * 120.0f - 60.0f);
            } while (!(bl2 = (d4 = this.s - this.h) * d4 + (d3 = this.t - this.i) * d3 + (d2 = this.u - this.bm) * d2 > 100.0));
            this.bD = null;
        }
    }

    private float b(double d2) {
        return (float)qh.g(d2);
    }

    private boolean a(azt azt2) {
        int n2 = qh.c(azt2.a);
        int n3 = qh.c(azt2.b);
        int n4 = qh.c(azt2.c);
        int n5 = qh.c(azt2.d);
        int n6 = qh.c(azt2.e);
        int n7 = qh.c(azt2.f);
        boolean bl2 = false;
        boolean bl3 = false;
        for (int i2 = n2; i2 <= n5; ++i2) {
            for (int i3 = n3; i3 <= n6; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    aji aji2 = this.o.a(i2, i3, i4);
                    if (aji2.o() == awt.a) continue;
                    if (aji2 == ajn.Z || aji2 == ajn.bs || aji2 == ajn.h || !this.o.O().b("mobGriefing")) {
                        bl2 = true;
                        continue;
                    }
                    bl3 = this.o.f(i2, i3, i4) || bl3;
                }
            }
        }
        if (bl3) {
            double d2 = azt2.a + (azt2.d - azt2.a) * (double)this.Z.nextFloat();
            double d3 = azt2.b + (azt2.e - azt2.b) * (double)this.Z.nextFloat();
            double d4 = azt2.c + (azt2.f - azt2.c) * (double)this.Z.nextFloat();
            this.o.a("largeexplode", d2, d3, d4, 0.0, 0.0, 0.0);
        }
        return bl2;
    }

    @Override
    public boolean a(wy wy2, ro ro2, float f2) {
        if (wy2 != this.bq) {
            f2 = f2 / 4.0f + 1.0f;
        }
        float f3 = this.y * (float)Math.PI / 180.0f;
        float f4 = qh.a(f3);
        float f5 = qh.b(f3);
        this.h = this.s + (double)(f4 * 5.0f) + (double)((this.Z.nextFloat() - 0.5f) * 2.0f);
        this.i = this.t + (double)(this.Z.nextFloat() * 3.0f) + 1.0;
        this.bm = this.u - (double)(f5 * 5.0f) + (double)((this.Z.nextFloat() - 0.5f) * 2.0f);
        this.bD = null;
        if (ro2.j() instanceof yz || ro2.c()) {
            this.e(ro2, f2);
        }
        return true;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        return false;
    }

    protected boolean e(ro ro2, float f2) {
        return super.a(ro2, f2);
    }

    @Override
    protected void aF() {
        ++this.bB;
        if (this.bB >= 180 && this.bB <= 200) {
            float f2 = (this.Z.nextFloat() - 0.5f) * 8.0f;
            float f3 = (this.Z.nextFloat() - 0.5f) * 4.0f;
            float f4 = (this.Z.nextFloat() - 0.5f) * 8.0f;
            this.o.a("hugeexplosion", this.s + (double)f2, this.t + 2.0 + (double)f3, this.u + (double)f4, 0.0, 0.0, 0.0);
        }
        if (!this.o.E) {
            if (this.bB > 150 && this.bB % 5 == 0) {
                int n2;
                for (int i2 = 1000; i2 > 0; i2 -= n2) {
                    n2 = sq.a(i2);
                    this.o.d(new sq(this.o, this.s, this.t, this.u, n2));
                }
            }
            if (this.bB == 1) {
                this.o.b(1018, (int)this.s, (int)this.t, (int)this.u, 0);
            }
        }
        this.d(0.0, 0.1f, 0.0);
        this.aM = this.y += 20.0f;
        if (this.bB == 200 && !this.o.E) {
            int n3;
            for (int i3 = 2000; i3 > 0; i3 -= n3) {
                n3 = sq.a(i3);
                this.o.d(new sq(this.o, this.s, this.t, this.u, n3));
            }
            this.b(qh.c(this.s), qh.c(this.u));
            this.B();
        }
    }

    private void b(int n2, int n3) {
        int n4 = 64;
        akt.a = true;
        int n5 = 4;
        for (int i2 = n4 - 1; i2 <= n4 + 32; ++i2) {
            for (int i3 = n2 - n5; i3 <= n2 + n5; ++i3) {
                for (int i4 = n3 - n5; i4 <= n3 + n5; ++i4) {
                    double d2 = i3 - n2;
                    double d3 = i4 - n3;
                    double d4 = d2 * d2 + d3 * d3;
                    if (!(d4 <= ((double)n5 - 0.5) * ((double)n5 - 0.5))) continue;
                    if (i2 < n4) {
                        if (!(d4 <= ((double)(n5 - 1) - 0.5) * ((double)(n5 - 1) - 0.5))) continue;
                        this.o.b(i3, i2, i4, ajn.h);
                        continue;
                    }
                    if (i2 > n4) {
                        this.o.b(i3, i2, i4, ajn.a);
                        continue;
                    }
                    if (d4 > ((double)(n5 - 1) - 0.5) * ((double)(n5 - 1) - 0.5)) {
                        this.o.b(i3, i2, i4, ajn.h);
                        continue;
                    }
                    this.o.b(i3, i2, i4, ajn.bq);
                }
            }
        }
        this.o.b(n2, n4 + 0, n3, ajn.h);
        this.o.b(n2, n4 + 1, n3, ajn.h);
        this.o.b(n2, n4 + 2, n3, ajn.h);
        this.o.b(n2 - 1, n4 + 2, n3, ajn.aa);
        this.o.b(n2 + 1, n4 + 2, n3, ajn.aa);
        this.o.b(n2, n4 + 2, n3 - 1, ajn.aa);
        this.o.b(n2, n4 + 2, n3 + 1, ajn.aa);
        this.o.b(n2, n4 + 3, n3, ajn.h);
        this.o.b(n2, n4 + 4, n3, ajn.bt);
        akt.a = false;
    }

    @Override
    protected void w() {
    }

    @Override
    public sa[] at() {
        return this.bp;
    }

    @Override
    public boolean R() {
        return false;
    }

    @Override
    public ahb a() {
        return this.o;
    }

    @Override
    protected String t() {
        return "mob.enderdragon.growl";
    }

    @Override
    protected String aT() {
        return "mob.enderdragon.hit";
    }

    @Override
    protected float bf() {
        return 5.0f;
    }
}

