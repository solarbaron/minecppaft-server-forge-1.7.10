/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aqv
implements apu {
    private Random i;
    private awk j;
    private awk k;
    private awk l;
    private awk m;
    private awk n;
    public awk a;
    public awk b;
    private ahb o;
    private double[] p;
    public atd c = new atd();
    private double[] q = new double[256];
    private double[] r = new double[256];
    private double[] s = new double[256];
    private aqx t = new aqy();
    double[] d;
    double[] e;
    double[] f;
    double[] g;
    double[] h;

    public aqv(ahb ahb2, long l2) {
        this.o = ahb2;
        this.i = new Random(l2);
        this.j = new awk(this.i, 16);
        this.k = new awk(this.i, 16);
        this.l = new awk(this.i, 8);
        this.m = new awk(this.i, 4);
        this.n = new awk(this.i, 4);
        this.a = new awk(this.i, 10);
        this.b = new awk(this.i, 16);
    }

    public void a(int n2, int n3, aji[] ajiArray) {
        int n4 = 4;
        int n5 = 32;
        int n6 = n4 + 1;
        int n7 = 17;
        int n8 = n4 + 1;
        this.p = this.a(this.p, n2 * n4, 0, n3 * n4, n6, n7, n8);
        for (int i2 = 0; i2 < n4; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                for (int i4 = 0; i4 < 16; ++i4) {
                    double d2 = 0.125;
                    double d3 = this.p[((i2 + 0) * n8 + (i3 + 0)) * n7 + (i4 + 0)];
                    double d4 = this.p[((i2 + 0) * n8 + (i3 + 1)) * n7 + (i4 + 0)];
                    double d5 = this.p[((i2 + 1) * n8 + (i3 + 0)) * n7 + (i4 + 0)];
                    double d6 = this.p[((i2 + 1) * n8 + (i3 + 1)) * n7 + (i4 + 0)];
                    double d7 = (this.p[((i2 + 0) * n8 + (i3 + 0)) * n7 + (i4 + 1)] - d3) * d2;
                    double d8 = (this.p[((i2 + 0) * n8 + (i3 + 1)) * n7 + (i4 + 1)] - d4) * d2;
                    double d9 = (this.p[((i2 + 1) * n8 + (i3 + 0)) * n7 + (i4 + 1)] - d5) * d2;
                    double d10 = (this.p[((i2 + 1) * n8 + (i3 + 1)) * n7 + (i4 + 1)] - d6) * d2;
                    for (int i5 = 0; i5 < 8; ++i5) {
                        double d11 = 0.25;
                        double d12 = d3;
                        double d13 = d4;
                        double d14 = (d5 - d3) * d11;
                        double d15 = (d6 - d4) * d11;
                        for (int i6 = 0; i6 < 4; ++i6) {
                            int n9 = i6 + i2 * 4 << 11 | 0 + i3 * 4 << 7 | i4 * 8 + i5;
                            int n10 = 128;
                            double d16 = 0.25;
                            double d17 = d12;
                            double d18 = (d13 - d12) * d16;
                            for (int i7 = 0; i7 < 4; ++i7) {
                                aji aji2 = null;
                                if (i4 * 8 + i5 < n5) {
                                    aji2 = ajn.l;
                                }
                                if (d17 > 0.0) {
                                    aji2 = ajn.aL;
                                }
                                ajiArray[n9] = aji2;
                                n9 += n10;
                                d17 += d18;
                            }
                            d12 += d14;
                            d13 += d15;
                        }
                        d3 += d7;
                        d4 += d8;
                        d5 += d9;
                        d6 += d10;
                    }
                }
            }
        }
    }

    public void b(int n2, int n3, aji[] ajiArray) {
        int n4 = 64;
        double d2 = 0.03125;
        this.q = this.m.a(this.q, n2 * 16, n3 * 16, 0, 16, 16, 1, d2, d2, 1.0);
        this.r = this.m.a(this.r, n2 * 16, 109, n3 * 16, 16, 1, 16, d2, 1.0, d2);
        this.s = this.n.a(this.s, n2 * 16, n3 * 16, 0, 16, 16, 1, d2 * 2.0, d2 * 2.0, d2 * 2.0);
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                boolean bl2 = this.q[i2 + i3 * 16] + this.i.nextDouble() * 0.2 > 0.0;
                boolean bl3 = this.r[i2 + i3 * 16] + this.i.nextDouble() * 0.2 > 0.0;
                int n5 = (int)(this.s[i2 + i3 * 16] / 3.0 + 3.0 + this.i.nextDouble() * 0.25);
                int n6 = -1;
                aji aji2 = ajn.aL;
                aji aji3 = ajn.aL;
                for (int i4 = 127; i4 >= 0; --i4) {
                    int n7 = (i3 * 16 + i2) * 128 + i4;
                    if (i4 >= 127 - this.i.nextInt(5) || i4 <= 0 + this.i.nextInt(5)) {
                        ajiArray[n7] = ajn.h;
                        continue;
                    }
                    aji aji4 = ajiArray[n7];
                    if (aji4 == null || aji4.o() == awt.a) {
                        n6 = -1;
                        continue;
                    }
                    if (aji4 != ajn.aL) continue;
                    if (n6 == -1) {
                        if (n5 <= 0) {
                            aji2 = null;
                            aji3 = ajn.aL;
                        } else if (i4 >= n4 - 4 && i4 <= n4 + 1) {
                            aji2 = ajn.aL;
                            aji3 = ajn.aL;
                            if (bl3) {
                                aji2 = ajn.n;
                                aji3 = ajn.aL;
                            }
                            if (bl2) {
                                aji2 = ajn.aM;
                                aji3 = ajn.aM;
                            }
                        }
                        if (i4 < n4 && (aji2 == null || aji2.o() == awt.a)) {
                            aji2 = ajn.l;
                        }
                        n6 = n5;
                        if (i4 >= n4 - 1) {
                            ajiArray[n7] = aji2;
                            continue;
                        }
                        ajiArray[n7] = aji3;
                        continue;
                    }
                    if (n6 <= 0) continue;
                    --n6;
                    ajiArray[n7] = aji3;
                }
            }
        }
    }

    @Override
    public apx c(int n2, int n3) {
        return this.d(n2, n3);
    }

    @Override
    public apx d(int n2, int n3) {
        this.i.setSeed((long)n2 * 341873128712L + (long)n3 * 132897987541L);
        aji[] ajiArray = new aji[32768];
        this.a(n2, n3, ajiArray);
        this.b(n2, n3, ajiArray);
        this.t.a(this, this.o, n2, n3, ajiArray);
        this.c.a(this, this.o, n2, n3, ajiArray);
        apx apx2 = new apx(this.o, ajiArray, n2, n3);
        ahu[] ahuArray = this.o.v().b(null, n2 * 16, n3 * 16, 16, 16);
        byte[] byArray = apx2.m();
        for (int i2 = 0; i2 < byArray.length; ++i2) {
            byArray[i2] = (byte)ahuArray[i2].ay;
        }
        apx2.n();
        return apx2;
    }

    private double[] a(double[] dArray, int n2, int n3, int n4, int n5, int n6, int n7) {
        int n8;
        if (dArray == null) {
            dArray = new double[n5 * n6 * n7];
        }
        double d2 = 684.412;
        double d3 = 2053.236;
        this.g = this.a.a(this.g, n2, n3, n4, n5, 1, n7, 1.0, 0.0, 1.0);
        this.h = this.b.a(this.h, n2, n3, n4, n5, 1, n7, 100.0, 0.0, 100.0);
        this.d = this.l.a(this.d, n2, n3, n4, n5, n6, n7, d2 / 80.0, d3 / 60.0, d2 / 80.0);
        this.e = this.j.a(this.e, n2, n3, n4, n5, n6, n7, d2, d3, d2);
        this.f = this.k.a(this.f, n2, n3, n4, n5, n6, n7, d2, d3, d2);
        int n9 = 0;
        int n10 = 0;
        double[] dArray2 = new double[n6];
        for (n8 = 0; n8 < n6; ++n8) {
            dArray2[n8] = Math.cos((double)n8 * Math.PI * 6.0 / (double)n6) * 2.0;
            double d4 = n8;
            if (n8 > n6 / 2) {
                d4 = n6 - 1 - n8;
            }
            if (!(d4 < 4.0)) continue;
            d4 = 4.0 - d4;
            int n11 = n8;
            dArray2[n11] = dArray2[n11] - d4 * d4 * d4 * 10.0;
        }
        for (n8 = 0; n8 < n5; ++n8) {
            for (int i2 = 0; i2 < n7; ++i2) {
                double d5 = (this.g[n10] + 256.0) / 512.0;
                if (d5 > 1.0) {
                    d5 = 1.0;
                }
                double d6 = 0.0;
                double d7 = this.h[n10] / 8000.0;
                if (d7 < 0.0) {
                    d7 = -d7;
                }
                if ((d7 = d7 * 3.0 - 3.0) < 0.0) {
                    if ((d7 /= 2.0) < -1.0) {
                        d7 = -1.0;
                    }
                    d7 /= 1.4;
                    d7 /= 2.0;
                    d5 = 0.0;
                } else {
                    if (d7 > 1.0) {
                        d7 = 1.0;
                    }
                    d7 /= 6.0;
                }
                d5 += 0.5;
                d7 = d7 * (double)n6 / 16.0;
                ++n10;
                for (int i3 = 0; i3 < n6; ++i3) {
                    double d8;
                    double d9 = 0.0;
                    double d10 = dArray2[i3];
                    double d11 = this.e[n9] / 512.0;
                    double d12 = this.f[n9] / 512.0;
                    double d13 = (this.d[n9] / 10.0 + 1.0) / 2.0;
                    d9 = d13 < 0.0 ? d11 : (d13 > 1.0 ? d12 : d11 + (d12 - d11) * d13);
                    d9 -= d10;
                    if (i3 > n6 - 4) {
                        d8 = (float)(i3 - (n6 - 4)) / 3.0f;
                        d9 = d9 * (1.0 - d8) + -10.0 * d8;
                    }
                    if ((double)i3 < d6) {
                        d8 = (d6 - (double)i3) / 4.0;
                        if (d8 < 0.0) {
                            d8 = 0.0;
                        }
                        if (d8 > 1.0) {
                            d8 = 1.0;
                        }
                        d9 = d9 * (1.0 - d8) + -10.0 * d8;
                    }
                    dArray[n9] = d9;
                    ++n9;
                }
            }
        }
        return dArray;
    }

    @Override
    public boolean a(int n2, int n3) {
        return true;
    }

    @Override
    public void a(apu apu2, int n2, int n3) {
        int n4;
        int n5;
        int n6;
        int n7;
        int n8;
        int n9;
        akx.M = true;
        int n10 = n2 * 16;
        int n11 = n3 * 16;
        this.c.a(this.o, this.i, n2, n3);
        for (n9 = 0; n9 < 8; ++n9) {
            n8 = n10 + this.i.nextInt(16) + 8;
            n7 = this.i.nextInt(120) + 4;
            n6 = n11 + this.i.nextInt(16) + 8;
            new ars(ajn.k, false).a(this.o, this.i, n8, n7, n6);
        }
        n9 = this.i.nextInt(this.i.nextInt(10) + 1) + 1;
        for (n8 = 0; n8 < n9; ++n8) {
            n7 = n10 + this.i.nextInt(16) + 8;
            n6 = this.i.nextInt(120) + 4;
            n5 = n11 + this.i.nextInt(16) + 8;
            new arq().a(this.o, this.i, n7, n6, n5);
        }
        n9 = this.i.nextInt(this.i.nextInt(10) + 1);
        for (n8 = 0; n8 < n9; ++n8) {
            n7 = n10 + this.i.nextInt(16) + 8;
            n6 = this.i.nextInt(120) + 4;
            n5 = n11 + this.i.nextInt(16) + 8;
            new ary().a(this.o, this.i, n7, n6, n5);
        }
        for (n8 = 0; n8 < 10; ++n8) {
            n7 = n10 + this.i.nextInt(16) + 8;
            n6 = this.i.nextInt(128);
            n5 = n11 + this.i.nextInt(16) + 8;
            new arr().a(this.o, this.i, n7, n6, n5);
        }
        if (this.i.nextInt(1) == 0) {
            n8 = n10 + this.i.nextInt(16) + 8;
            n7 = this.i.nextInt(128);
            n6 = n11 + this.i.nextInt(16) + 8;
            new aro(ajn.P).a(this.o, this.i, n8, n7, n6);
        }
        if (this.i.nextInt(1) == 0) {
            n8 = n10 + this.i.nextInt(16) + 8;
            n7 = this.i.nextInt(128);
            n6 = n11 + this.i.nextInt(16) + 8;
            new aro(ajn.Q).a(this.o, this.i, n8, n7, n6);
        }
        ase ase2 = new ase(ajn.bY, 13, ajn.aL);
        for (n7 = 0; n7 < 16; ++n7) {
            n6 = n10 + this.i.nextInt(16);
            n5 = this.i.nextInt(108) + 10;
            n4 = n11 + this.i.nextInt(16);
            ((arn)ase2).a(this.o, this.i, n6, n5, n4);
        }
        for (n7 = 0; n7 < 16; ++n7) {
            n6 = n10 + this.i.nextInt(16);
            n5 = this.i.nextInt(108) + 10;
            n4 = n11 + this.i.nextInt(16);
            new ars(ajn.k, true).a(this.o, this.i, n6, n5, n4);
        }
        akx.M = false;
    }

    @Override
    public boolean a(boolean bl2, qk qk2) {
        return true;
    }

    @Override
    public void c() {
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean e() {
        return true;
    }

    @Override
    public String f() {
        return "HellRandomLevelSource";
    }

    @Override
    public List a(sx sx2, int n2, int n3, int n4) {
        if (sx2 == sx.a) {
            if (this.c.b(n2, n3, n4)) {
                return this.c.b();
            }
            if (this.c.d(n2, n3, n4) && this.o.a(n2, n3 - 1, n4) == ajn.bj) {
                return this.c.b();
            }
        }
        ahu ahu2 = this.o.a(n2, n4);
        return ahu2.a(sx2);
    }

    @Override
    public agt a(ahb ahb2, String string, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public int g() {
        return 0;
    }

    @Override
    public void e(int n2, int n3) {
        this.c.a(this, this.o, n2, n3, null);
    }
}

