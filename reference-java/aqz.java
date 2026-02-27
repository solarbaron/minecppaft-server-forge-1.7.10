/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aqz
implements apu {
    private Random i;
    private awk j;
    private awk k;
    private awk l;
    private awl m;
    public awk a;
    public awk b;
    public awk c;
    private ahb n;
    private final boolean o;
    private ahm p;
    private final double[] q;
    private final float[] r;
    private double[] s = new double[256];
    private aqx t = new aqw();
    private aug u = new aug();
    private avn v = new avn();
    private asw w = new asw();
    private atx x = new atx();
    private aqx y = new aqs();
    private ahu[] z;
    double[] d;
    double[] e;
    double[] f;
    double[] g;
    int[][] h = new int[32][32];

    public aqz(ahb ahb2, long l2, boolean bl2) {
        this.n = ahb2;
        this.o = bl2;
        this.p = ahb2.N().u();
        this.i = new Random(l2);
        this.j = new awk(this.i, 16);
        this.k = new awk(this.i, 16);
        this.l = new awk(this.i, 8);
        this.m = new awl(this.i, 4);
        this.a = new awk(this.i, 10);
        this.b = new awk(this.i, 16);
        this.c = new awk(this.i, 8);
        this.q = new double[825];
        this.r = new float[25];
        for (int i2 = -2; i2 <= 2; ++i2) {
            for (int i3 = -2; i3 <= 2; ++i3) {
                float f2;
                this.r[i2 + 2 + (i3 + 2) * 5] = f2 = 10.0f / qh.c((float)(i2 * i2 + i3 * i3) + 0.2f);
            }
        }
    }

    public void a(int n2, int n3, aji[] ajiArray) {
        int n4 = 63;
        this.z = this.n.v().a(this.z, n2 * 4 - 2, n3 * 4 - 2, 10, 10);
        this.a(n2 * 4, 0, n3 * 4);
        for (int i2 = 0; i2 < 4; ++i2) {
            int n5 = i2 * 5;
            int n6 = (i2 + 1) * 5;
            for (int i3 = 0; i3 < 4; ++i3) {
                int n7 = (n5 + i3) * 33;
                int n8 = (n5 + i3 + 1) * 33;
                int n9 = (n6 + i3) * 33;
                int n10 = (n6 + i3 + 1) * 33;
                for (int i4 = 0; i4 < 32; ++i4) {
                    double d2 = 0.125;
                    double d3 = this.q[n7 + i4];
                    double d4 = this.q[n8 + i4];
                    double d5 = this.q[n9 + i4];
                    double d6 = this.q[n10 + i4];
                    double d7 = (this.q[n7 + i4 + 1] - d3) * d2;
                    double d8 = (this.q[n8 + i4 + 1] - d4) * d2;
                    double d9 = (this.q[n9 + i4 + 1] - d5) * d2;
                    double d10 = (this.q[n10 + i4 + 1] - d6) * d2;
                    for (int i5 = 0; i5 < 8; ++i5) {
                        double d11 = 0.25;
                        double d12 = d3;
                        double d13 = d4;
                        double d14 = (d5 - d3) * d11;
                        double d15 = (d6 - d4) * d11;
                        for (int i6 = 0; i6 < 4; ++i6) {
                            int n11 = i6 + i2 * 4 << 12 | 0 + i3 * 4 << 8 | i4 * 8 + i5;
                            int n12 = 256;
                            n11 -= n12;
                            double d16 = 0.25;
                            double d17 = d12;
                            double d18 = (d13 - d12) * d16;
                            d17 -= d18;
                            for (int i7 = 0; i7 < 4; ++i7) {
                                double d19;
                                d17 += d18;
                                ajiArray[n11 += n12] = d19 > 0.0 ? ajn.b : (i4 * 8 + i5 < n4 ? ajn.j : null);
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

    public void a(int n2, int n3, aji[] ajiArray, byte[] byArray, ahu[] ahuArray) {
        double d2 = 0.03125;
        this.s = this.m.a(this.s, n2 * 16, n3 * 16, 16, 16, d2 * 2.0, d2 * 2.0, 1.0);
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                ahu ahu2 = ahuArray[i3 + i2 * 16];
                ahu2.a(this.n, this.i, ajiArray, byArray, n2 * 16 + i2, n3 * 16 + i3, this.s[i3 + i2 * 16]);
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
        aji[] ajiArray = new aji[65536];
        byte[] byArray = new byte[65536];
        this.a(n2, n3, ajiArray);
        this.z = this.n.v().b(this.z, n2 * 16, n3 * 16, 16, 16);
        this.a(n2, n3, ajiArray, byArray, this.z);
        this.t.a(this, this.n, n2, n3, ajiArray);
        this.y.a(this, this.n, n2, n3, ajiArray);
        if (this.o) {
            this.w.a(this, this.n, n2, n3, ajiArray);
            this.v.a(this, this.n, n2, n3, ajiArray);
            this.u.a(this, this.n, n2, n3, ajiArray);
            this.x.a(this, this.n, n2, n3, ajiArray);
        }
        apx apx2 = new apx(this.n, ajiArray, byArray, n2, n3);
        byte[] byArray2 = apx2.m();
        for (int i2 = 0; i2 < byArray2.length; ++i2) {
            byArray2[i2] = (byte)this.z[i2].ay;
        }
        apx2.b();
        return apx2;
    }

    private void a(int n2, int n3, int n4) {
        double d2 = 684.412;
        double d3 = 684.412;
        double d4 = 512.0;
        double d5 = 512.0;
        this.g = this.b.a(this.g, n2, n4, 5, 5, 200.0, 200.0, 0.5);
        this.d = this.l.a(this.d, n2, n3, n4, 5, 33, 5, 8.555150000000001, 4.277575000000001, 8.555150000000001);
        this.e = this.j.a(this.e, n2, n3, n4, 5, 33, 5, 684.412, 684.412, 684.412);
        this.f = this.k.a(this.f, n2, n3, n4, 5, 33, 5, 684.412, 684.412, 684.412);
        n4 = 0;
        n2 = 0;
        int n5 = 0;
        int n6 = 0;
        double d6 = 8.5;
        for (int i2 = 0; i2 < 5; ++i2) {
            for (int i3 = 0; i3 < 5; ++i3) {
                float f2 = 0.0f;
                float f3 = 0.0f;
                float f4 = 0.0f;
                int n7 = 2;
                ahu ahu2 = this.z[i2 + 2 + (i3 + 2) * 10];
                for (int i4 = -n7; i4 <= n7; ++i4) {
                    for (int i5 = -n7; i5 <= n7; ++i5) {
                        ahu ahu3 = this.z[i2 + i4 + 2 + (i3 + i5 + 2) * 10];
                        float f5 = ahu3.am;
                        float f6 = ahu3.an;
                        if (this.p == ahm.e && f5 > 0.0f) {
                            f5 = 1.0f + f5 * 2.0f;
                            f6 = 1.0f + f6 * 4.0f;
                        }
                        float f7 = this.r[i4 + 2 + (i5 + 2) * 5] / (f5 + 2.0f);
                        if (ahu3.am > ahu2.am) {
                            f7 /= 2.0f;
                        }
                        f2 += f6 * f7;
                        f3 += f5 * f7;
                        f4 += f7;
                    }
                }
                f2 /= f4;
                f3 /= f4;
                f2 = f2 * 0.9f + 0.1f;
                f3 = (f3 * 4.0f - 1.0f) / 8.0f;
                double d7 = this.g[n6] / 8000.0;
                if (d7 < 0.0) {
                    d7 = -d7 * 0.3;
                }
                if ((d7 = d7 * 3.0 - 2.0) < 0.0) {
                    if ((d7 /= 2.0) < -1.0) {
                        d7 = -1.0;
                    }
                    d7 /= 1.4;
                    d7 /= 2.0;
                } else {
                    if (d7 > 1.0) {
                        d7 = 1.0;
                    }
                    d7 /= 8.0;
                }
                ++n6;
                double d8 = f3;
                double d9 = f2;
                d8 += d7 * 0.2;
                d8 = d8 * 8.5 / 8.0;
                double d10 = 8.5 + d8 * 4.0;
                for (int i6 = 0; i6 < 33; ++i6) {
                    double d11 = ((double)i6 - d10) * 12.0 * 128.0 / 256.0 / d9;
                    if (d11 < 0.0) {
                        d11 *= 4.0;
                    }
                    double d12 = this.e[n5] / 512.0;
                    double d13 = this.f[n5] / 512.0;
                    double d14 = (this.d[n5] / 10.0 + 1.0) / 2.0;
                    double d15 = qh.b(d12, d13, d14) - d11;
                    if (i6 > 29) {
                        double d16 = (float)(i6 - 29) / 3.0f;
                        d15 = d15 * (1.0 - d16) + -10.0 * d16;
                    }
                    this.q[n5] = d15;
                    ++n5;
                }
            }
        }
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
        akx.M = true;
        int n7 = n2 * 16;
        int n8 = n3 * 16;
        ahu ahu2 = this.n.a(n7 + 16, n8 + 16);
        this.i.setSeed(this.n.H());
        long l2 = this.i.nextLong() / 2L * 2L + 1L;
        long l3 = this.i.nextLong() / 2L * 2L + 1L;
        this.i.setSeed((long)n2 * l2 + (long)n3 * l3 ^ this.n.H());
        boolean bl2 = false;
        if (this.o) {
            this.w.a(this.n, this.i, n2, n3);
            bl2 = this.v.a(this.n, this.i, n2, n3);
            this.u.a(this.n, this.i, n2, n3);
            this.x.a(this.n, this.i, n2, n3);
        }
        if (ahu2 != ahu.q && ahu2 != ahu.F && !bl2 && this.i.nextInt(4) == 0) {
            n6 = n7 + this.i.nextInt(16) + 8;
            n5 = this.i.nextInt(256);
            n4 = n8 + this.i.nextInt(16) + 8;
            new arx(ajn.j).a(this.n, this.i, n6, n5, n4);
        }
        if (!bl2 && this.i.nextInt(8) == 0) {
            n6 = n7 + this.i.nextInt(16) + 8;
            n5 = this.i.nextInt(this.i.nextInt(248) + 8);
            n4 = n8 + this.i.nextInt(16) + 8;
            if (n5 < 63 || this.i.nextInt(10) == 0) {
                new arx(ajn.l).a(this.n, this.i, n6, n5, n4);
            }
        }
        for (n6 = 0; n6 < 8; ++n6) {
            n5 = n7 + this.i.nextInt(16) + 8;
            n4 = this.i.nextInt(256);
            int n9 = n8 + this.i.nextInt(16) + 8;
            new asd().a(this.n, this.i, n5, n4, n9);
        }
        ahu2.a(this.n, this.i, n7, n8);
        aho.a(this.n, ahu2, n7 + 8, n8 + 8, 16, 16, this.i);
        n7 += 8;
        n8 += 8;
        for (n6 = 0; n6 < 16; ++n6) {
            for (n5 = 0; n5 < 16; ++n5) {
                n4 = this.n.h(n7 + n6, n8 + n5);
                if (this.n.r(n6 + n7, n4 - 1, n5 + n8)) {
                    this.n.d(n6 + n7, n4 - 1, n5 + n8, ajn.aD, 0, 2);
                }
                if (!this.n.e(n6 + n7, n4, n5 + n8, true)) continue;
                this.n.d(n6 + n7, n4, n5 + n8, ajn.aC, 0, 2);
            }
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
        return "RandomLevelSource";
    }

    @Override
    public List a(sx sx2, int n2, int n3, int n4) {
        ahu ahu2 = this.n.a(n2, n4);
        if (sx2 == sx.a && this.x.a(n2, n3, n4)) {
            return this.x.b();
        }
        return ahu2.a(sx2);
    }

    @Override
    public agt a(ahb ahb2, String string, int n2, int n3, int n4) {
        if ("Stronghold".equals(string) && this.u != null) {
            return this.u.a(ahb2, n2, n3, n4);
        }
        return null;
    }

    @Override
    public int g() {
        return 0;
    }

    @Override
    public void e(int n2, int n3) {
        if (this.o) {
            this.w.a(this, this.n, n2, n3, null);
            this.v.a(this, this.n, n2, n3, null);
            this.u.a(this, this.n, n2, n3, null);
            this.x.a(this, this.n, n2, n3, null);
        }
    }
}

