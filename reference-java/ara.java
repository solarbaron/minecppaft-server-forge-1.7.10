/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ara
implements apu {
    private Random i;
    private awk j;
    private awk k;
    private awk l;
    public awk a;
    public awk b;
    private ahb m;
    private double[] n;
    private ahu[] o;
    double[] c;
    double[] d;
    double[] e;
    double[] f;
    double[] g;
    int[][] h = new int[32][32];

    public ara(ahb ahb2, long l2) {
        this.m = ahb2;
        this.i = new Random(l2);
        this.j = new awk(this.i, 16);
        this.k = new awk(this.i, 16);
        this.l = new awk(this.i, 8);
        this.a = new awk(this.i, 10);
        this.b = new awk(this.i, 16);
    }

    public void a(int n2, int n3, aji[] ajiArray, ahu[] ahuArray) {
        int n4 = 2;
        int n5 = n4 + 1;
        int n6 = 33;
        int n7 = n4 + 1;
        this.n = this.a(this.n, n2 * n4, 0, n3 * n4, n5, n6, n7);
        for (int i2 = 0; i2 < n4; ++i2) {
            for (int i3 = 0; i3 < n4; ++i3) {
                for (int i4 = 0; i4 < 32; ++i4) {
                    double d2 = 0.25;
                    double d3 = this.n[((i2 + 0) * n7 + (i3 + 0)) * n6 + (i4 + 0)];
                    double d4 = this.n[((i2 + 0) * n7 + (i3 + 1)) * n6 + (i4 + 0)];
                    double d5 = this.n[((i2 + 1) * n7 + (i3 + 0)) * n6 + (i4 + 0)];
                    double d6 = this.n[((i2 + 1) * n7 + (i3 + 1)) * n6 + (i4 + 0)];
                    double d7 = (this.n[((i2 + 0) * n7 + (i3 + 0)) * n6 + (i4 + 1)] - d3) * d2;
                    double d8 = (this.n[((i2 + 0) * n7 + (i3 + 1)) * n6 + (i4 + 1)] - d4) * d2;
                    double d9 = (this.n[((i2 + 1) * n7 + (i3 + 0)) * n6 + (i4 + 1)] - d5) * d2;
                    double d10 = (this.n[((i2 + 1) * n7 + (i3 + 1)) * n6 + (i4 + 1)] - d6) * d2;
                    for (int i5 = 0; i5 < 4; ++i5) {
                        double d11 = 0.125;
                        double d12 = d3;
                        double d13 = d4;
                        double d14 = (d5 - d3) * d11;
                        double d15 = (d6 - d4) * d11;
                        for (int i6 = 0; i6 < 8; ++i6) {
                            int n8 = i6 + i2 * 8 << 11 | 0 + i3 * 8 << 7 | i4 * 4 + i5;
                            int n9 = 128;
                            double d16 = 0.125;
                            double d17 = d12;
                            double d18 = (d13 - d12) * d16;
                            for (int i7 = 0; i7 < 8; ++i7) {
                                aji aji2 = null;
                                if (d17 > 0.0) {
                                    aji2 = ajn.bs;
                                }
                                ajiArray[n8] = aji2;
                                n8 += n9;
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

    public void b(int n2, int n3, aji[] ajiArray, ahu[] ahuArray) {
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                int n4 = 1;
                int n5 = -1;
                aji aji2 = ajn.bs;
                aji aji3 = ajn.bs;
                for (int i4 = 127; i4 >= 0; --i4) {
                    int n6 = (i3 * 16 + i2) * 128 + i4;
                    aji aji4 = ajiArray[n6];
                    if (aji4 == null || aji4.o() == awt.a) {
                        n5 = -1;
                        continue;
                    }
                    if (aji4 != ajn.b) continue;
                    if (n5 == -1) {
                        if (n4 <= 0) {
                            aji2 = null;
                            aji3 = ajn.bs;
                        }
                        n5 = n4;
                        if (i4 >= 0) {
                            ajiArray[n6] = aji2;
                            continue;
                        }
                        ajiArray[n6] = aji3;
                        continue;
                    }
                    if (n5 <= 0) continue;
                    --n5;
                    ajiArray[n6] = aji3;
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
        this.o = this.m.v().b(this.o, n2 * 16, n3 * 16, 16, 16);
        this.a(n2, n3, ajiArray, this.o);
        this.b(n2, n3, ajiArray, this.o);
        apx apx2 = new apx(this.m, ajiArray, n2, n3);
        byte[] byArray = apx2.m();
        for (int i2 = 0; i2 < byArray.length; ++i2) {
            byArray[i2] = (byte)this.o[i2].ay;
        }
        apx2.b();
        return apx2;
    }

    private double[] a(double[] dArray, int n2, int n3, int n4, int n5, int n6, int n7) {
        if (dArray == null) {
            dArray = new double[n5 * n6 * n7];
        }
        double d2 = 684.412;
        double d3 = 684.412;
        this.f = this.a.a(this.f, n2, n4, n5, n7, 1.121, 1.121, 0.5);
        this.g = this.b.a(this.g, n2, n4, n5, n7, 200.0, 200.0, 0.5);
        this.c = this.l.a(this.c, n2, n3, n4, n5, n6, n7, (d2 *= 2.0) / 80.0, d3 / 160.0, d2 / 80.0);
        this.d = this.j.a(this.d, n2, n3, n4, n5, n6, n7, d2, d3, d2);
        this.e = this.k.a(this.e, n2, n3, n4, n5, n6, n7, d2, d3, d2);
        int n8 = 0;
        int n9 = 0;
        for (int i2 = 0; i2 < n5; ++i2) {
            for (int i3 = 0; i3 < n7; ++i3) {
                double d4;
                double d5 = (this.f[n9] + 256.0) / 512.0;
                if (d5 > 1.0) {
                    d5 = 1.0;
                }
                if ((d4 = this.g[n9] / 8000.0) < 0.0) {
                    d4 = -d4 * 0.3;
                }
                d4 = d4 * 3.0 - 2.0;
                float f2 = (float)(i2 + n2 - 0) / 1.0f;
                float f3 = (float)(i3 + n4 - 0) / 1.0f;
                float f4 = 100.0f - qh.c(f2 * f2 + f3 * f3) * 8.0f;
                if (f4 > 80.0f) {
                    f4 = 80.0f;
                }
                if (f4 < -100.0f) {
                    f4 = -100.0f;
                }
                if (d4 > 1.0) {
                    d4 = 1.0;
                }
                d4 /= 8.0;
                d4 = 0.0;
                if (d5 < 0.0) {
                    d5 = 0.0;
                }
                d5 += 0.5;
                d4 = d4 * (double)n6 / 16.0;
                ++n9;
                double d6 = (double)n6 / 2.0;
                for (int i4 = 0; i4 < n6; ++i4) {
                    double d7;
                    double d8 = 0.0;
                    double d9 = ((double)i4 - d6) * 8.0 / d5;
                    if (d9 < 0.0) {
                        d9 *= -1.0;
                    }
                    double d10 = this.d[n8] / 512.0;
                    double d11 = this.e[n8] / 512.0;
                    double d12 = (this.c[n8] / 10.0 + 1.0) / 2.0;
                    d8 = d12 < 0.0 ? d10 : (d12 > 1.0 ? d11 : d10 + (d11 - d10) * d12);
                    d8 -= 8.0;
                    d8 += (double)f4;
                    int n10 = 2;
                    if (i4 > n6 / 2 - n10) {
                        d7 = (float)(i4 - (n6 / 2 - n10)) / 64.0f;
                        if (d7 < 0.0) {
                            d7 = 0.0;
                        }
                        if (d7 > 1.0) {
                            d7 = 1.0;
                        }
                        d8 = d8 * (1.0 - d7) + -3000.0 * d7;
                    }
                    if (i4 < (n10 = 8)) {
                        d7 = (float)(n10 - i4) / ((float)n10 - 1.0f);
                        d8 = d8 * (1.0 - d7) + -30.0 * d7;
                    }
                    dArray[n8] = d8;
                    ++n8;
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
        akx.M = true;
        int n4 = n2 * 16;
        int n5 = n3 * 16;
        ahu ahu2 = this.m.a(n4 + 16, n5 + 16);
        ahu2.a(this.m, this.m.s, n4, n5);
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
        return "RandomLevelSource";
    }

    @Override
    public List a(sx sx2, int n2, int n3, int n4) {
        ahu ahu2 = this.m.a(n2, n4);
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
    }
}

