/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

public class ahp {
    private final mt a;
    private final Random b;
    private final qd c = new qd();
    private final List d = new ArrayList();

    public ahp(mt mt2) {
        this.a = mt2;
        this.b = new Random(mt2.H());
    }

    public void a(sa sa2, double d2, double d3, double d4, float f2) {
        if (this.a.t.i == 1) {
            int n2 = qh.c(sa2.s);
            int n3 = qh.c(sa2.t) - 1;
            int n4 = qh.c(sa2.u);
            int n5 = 1;
            int n6 = 0;
            for (int i2 = -2; i2 <= 2; ++i2) {
                for (int i3 = -2; i3 <= 2; ++i3) {
                    for (int i4 = -1; i4 < 3; ++i4) {
                        int n7 = n2 + i3 * n5 + i2 * n6;
                        int n8 = n3 + i4;
                        int n9 = n4 + i3 * n6 - i2 * n5;
                        boolean bl2 = i4 < 0;
                        this.a.b(n7, n8, n9, bl2 ? ajn.Z : ajn.a);
                    }
                }
            }
            sa2.b(n2, n3, n4, sa2.y, 0.0f);
            sa2.x = 0.0;
            sa2.w = 0.0;
            sa2.v = 0.0;
            return;
        }
        if (this.b(sa2, d2, d3, d4, f2)) {
            return;
        }
        this.a(sa2);
        this.b(sa2, d2, d3, d4, f2);
    }

    public boolean b(sa sa2, double d2, double d3, double d4, float f2) {
        double d5;
        int n2 = 128;
        double d6 = -1.0;
        int n3 = 0;
        int n4 = 0;
        int n5 = 0;
        int n6 = qh.c(sa2.s);
        int n7 = qh.c(sa2.u);
        long l2 = agu.a(n6, n7);
        boolean bl2 = true;
        if (this.c.b(l2)) {
            ahq ahq2 = (ahq)this.c.a(l2);
            d6 = 0.0;
            n3 = ahq2.a;
            n4 = ahq2.b;
            n5 = ahq2.c;
            ahq2.d = this.a.I();
            bl2 = false;
        } else {
            for (int i2 = n6 - n2; i2 <= n6 + n2; ++i2) {
                double d7 = (double)i2 + 0.5 - sa2.s;
                for (int i3 = n7 - n2; i3 <= n7 + n2; ++i3) {
                    double d8 = (double)i3 + 0.5 - sa2.u;
                    for (int i4 = this.a.S() - 1; i4 >= 0; --i4) {
                        if (this.a.a(i2, i4, i3) != ajn.aO) continue;
                        while (this.a.a(i2, i4 - 1, i3) == ajn.aO) {
                            --i4;
                        }
                        d5 = (double)i4 + 0.5 - sa2.t;
                        double d9 = d7 * d7 + d5 * d5 + d8 * d8;
                        if (!(d6 < 0.0) && !(d9 < d6)) continue;
                        d6 = d9;
                        n3 = i2;
                        n4 = i4;
                        n5 = i3;
                    }
                }
            }
        }
        if (d6 >= 0.0) {
            int n8 = n3;
            int n9 = n4;
            int n10 = n5;
            if (bl2) {
                this.c.a(l2, new ahq(this, n8, n9, n10, this.a.I()));
                this.d.add(l2);
            }
            double d10 = (double)n8 + 0.5;
            double d11 = (double)n9 + 0.5;
            d5 = (double)n10 + 0.5;
            int n11 = -1;
            if (this.a.a(n8 - 1, n9, n10) == ajn.aO) {
                n11 = 2;
            }
            if (this.a.a(n8 + 1, n9, n10) == ajn.aO) {
                n11 = 0;
            }
            if (this.a.a(n8, n9, n10 - 1) == ajn.aO) {
                n11 = 3;
            }
            if (this.a.a(n8, n9, n10 + 1) == ajn.aO) {
                n11 = 1;
            }
            int n12 = sa2.ay();
            if (n11 > -1) {
                boolean bl3;
                int n13 = p.a[n11];
                int n14 = p.h[n11];
                int n15 = p.a[n14];
                int n16 = p.b[n11];
                int n17 = p.b[n14];
                boolean bl4 = !this.a.c(n8 + n13 + n15, n9, n10 + n16 + n17) || !this.a.c(n8 + n13 + n15, n9 + 1, n10 + n16 + n17);
                boolean bl5 = bl3 = !this.a.c(n8 + n13, n9, n10 + n16) || !this.a.c(n8 + n13, n9 + 1, n10 + n16);
                if (bl4 && bl3) {
                    n11 = p.f[n11];
                    n14 = p.f[n14];
                    n13 = p.a[n11];
                    n16 = p.b[n11];
                    n15 = p.a[n14];
                    n17 = p.b[n14];
                    d10 -= (double)n15;
                    d5 -= (double)n17;
                    bl4 = !this.a.c((n8 -= n15) + n13 + n15, n9, (n10 -= n17) + n16 + n17) || !this.a.c(n8 + n13 + n15, n9 + 1, n10 + n16 + n17);
                    bl3 = !this.a.c(n8 + n13, n9, n10 + n16) || !this.a.c(n8 + n13, n9 + 1, n10 + n16);
                }
                float f3 = 0.5f;
                float f4 = 0.5f;
                if (!bl4 && bl3) {
                    f3 = 1.0f;
                } else if (bl4 && !bl3) {
                    f3 = 0.0f;
                } else if (bl4 && bl3) {
                    f4 = 0.0f;
                }
                d10 += (double)((float)n15 * f3 + f4 * (float)n13);
                d5 += (double)((float)n17 * f3 + f4 * (float)n16);
                float f5 = 0.0f;
                float f6 = 0.0f;
                float f7 = 0.0f;
                float f8 = 0.0f;
                if (n11 == n12) {
                    f5 = 1.0f;
                    f6 = 1.0f;
                } else if (n11 == p.f[n12]) {
                    f5 = -1.0f;
                    f6 = -1.0f;
                } else if (n11 == p.g[n12]) {
                    f7 = 1.0f;
                    f8 = -1.0f;
                } else {
                    f7 = -1.0f;
                    f8 = 1.0f;
                }
                double d12 = sa2.v;
                double d13 = sa2.x;
                sa2.v = d12 * (double)f5 + d13 * (double)f8;
                sa2.x = d12 * (double)f7 + d13 * (double)f6;
                sa2.y = f2 - (float)(n12 * 90) + (float)(n11 * 90);
            } else {
                sa2.x = 0.0;
                sa2.w = 0.0;
                sa2.v = 0.0;
            }
            sa2.b(d10, d11, d5, sa2.y, sa2.z);
            return true;
        }
        return false;
    }

    public boolean a(sa sa2) {
        int n2;
        int n3;
        int n4;
        int n5;
        int n6;
        int n7;
        int n8;
        int n9;
        int n10;
        double d2;
        int n11;
        double d3;
        int n12;
        int n13 = 16;
        double d4 = -1.0;
        int n14 = qh.c(sa2.s);
        int n15 = qh.c(sa2.t);
        int n16 = qh.c(sa2.u);
        int n17 = n14;
        int n18 = n15;
        int n19 = n16;
        int n20 = 0;
        int n21 = this.b.nextInt(4);
        for (n12 = n14 - n13; n12 <= n14 + n13; ++n12) {
            d3 = (double)n12 + 0.5 - sa2.s;
            for (n11 = n16 - n13; n11 <= n16 + n13; ++n11) {
                d2 = (double)n11 + 0.5 - sa2.u;
                block2: for (n10 = this.a.S() - 1; n10 >= 0; --n10) {
                    if (!this.a.c(n12, n10, n11)) continue;
                    while (n10 > 0 && this.a.c(n12, n10 - 1, n11)) {
                        --n10;
                    }
                    for (n9 = n21; n9 < n21 + 4; ++n9) {
                        n8 = n9 % 2;
                        n7 = 1 - n8;
                        if (n9 % 4 >= 2) {
                            n8 = -n8;
                            n7 = -n7;
                        }
                        for (n6 = 0; n6 < 3; ++n6) {
                            for (n5 = 0; n5 < 4; ++n5) {
                                for (n4 = -1; n4 < 4; ++n4) {
                                    n3 = n12 + (n5 - 1) * n8 + n6 * n7;
                                    n2 = n10 + n4;
                                    int n22 = n11 + (n5 - 1) * n7 - n6 * n8;
                                    if (n4 < 0 && !this.a.a(n3, n2, n22).o().a() || n4 >= 0 && !this.a.c(n3, n2, n22)) continue block2;
                                }
                            }
                        }
                        double d5 = (double)n10 + 0.5 - sa2.t;
                        double d6 = d3 * d3 + d5 * d5 + d2 * d2;
                        if (!(d4 < 0.0) && !(d6 < d4)) continue;
                        d4 = d6;
                        n17 = n12;
                        n18 = n10;
                        n19 = n11;
                        n20 = n9 % 4;
                    }
                }
            }
        }
        if (d4 < 0.0) {
            for (n12 = n14 - n13; n12 <= n14 + n13; ++n12) {
                d3 = (double)n12 + 0.5 - sa2.s;
                for (n11 = n16 - n13; n11 <= n16 + n13; ++n11) {
                    d2 = (double)n11 + 0.5 - sa2.u;
                    block10: for (n10 = this.a.S() - 1; n10 >= 0; --n10) {
                        if (!this.a.c(n12, n10, n11)) continue;
                        while (n10 > 0 && this.a.c(n12, n10 - 1, n11)) {
                            --n10;
                        }
                        for (n9 = n21; n9 < n21 + 2; ++n9) {
                            n8 = n9 % 2;
                            n7 = 1 - n8;
                            for (int i2 = 0; i2 < 4; ++i2) {
                                for (n5 = -1; n5 < 4; ++n5) {
                                    int n23 = n12 + (i2 - 1) * n8;
                                    n3 = n10 + n5;
                                    n2 = n11 + (i2 - 1) * n7;
                                    if (n5 < 0 && !this.a.a(n23, n3, n2).o().a() || n5 >= 0 && !this.a.c(n23, n3, n2)) continue block10;
                                }
                            }
                            double d7 = (double)n10 + 0.5 - sa2.t;
                            double d8 = d3 * d3 + d7 * d7 + d2 * d2;
                            if (!(d4 < 0.0) && !(d8 < d4)) continue;
                            d4 = d8;
                            n17 = n12;
                            n18 = n10;
                            n19 = n11;
                            n20 = n9 % 2;
                        }
                    }
                }
            }
        }
        n12 = n20;
        int n24 = n17;
        int n25 = n18;
        n11 = n19;
        int n26 = n12 % 2;
        int n27 = 1 - n26;
        if (n12 % 4 >= 2) {
            n26 = -n26;
            n27 = -n27;
        }
        if (d4 < 0.0) {
            if (n18 < 70) {
                n18 = 70;
            }
            if (n18 > this.a.S() - 10) {
                n18 = this.a.S() - 10;
            }
            n25 = n18;
            for (n10 = -1; n10 <= 1; ++n10) {
                for (n9 = 1; n9 < 3; ++n9) {
                    for (n8 = -1; n8 < 3; ++n8) {
                        n7 = n24 + (n9 - 1) * n26 + n10 * n27;
                        n6 = n25 + n8;
                        n5 = n11 + (n9 - 1) * n27 - n10 * n26;
                        n4 = n8 < 0 ? 1 : 0;
                        this.a.b(n7, n6, n5, n4 != 0 ? ajn.Z : ajn.a);
                    }
                }
            }
        }
        for (n10 = 0; n10 < 4; ++n10) {
            for (n9 = 0; n9 < 4; ++n9) {
                for (n8 = -1; n8 < 4; ++n8) {
                    n7 = n24 + (n9 - 1) * n26;
                    n6 = n25 + n8;
                    n5 = n11 + (n9 - 1) * n27;
                    n4 = n9 == 0 || n9 == 3 || n8 == -1 || n8 == 3 ? 1 : 0;
                    this.a.d(n7, n6, n5, n4 != 0 ? ajn.Z : ajn.aO, 0, 2);
                }
            }
            for (n9 = 0; n9 < 4; ++n9) {
                for (n8 = -1; n8 < 4; ++n8) {
                    n7 = n24 + (n9 - 1) * n26;
                    n6 = n25 + n8;
                    n5 = n11 + (n9 - 1) * n27;
                    this.a.d(n7, n6, n5, this.a.a(n7, n6, n5));
                }
            }
        }
        return true;
    }

    public void a(long l2) {
        if (l2 % 100L == 0L) {
            Iterator iterator = this.d.iterator();
            long l3 = l2 - 600L;
            while (iterator.hasNext()) {
                Long l4 = (Long)iterator.next();
                ahq ahq2 = (ahq)this.c.a(l4);
                if (ahq2 != null && ahq2.d >= l3) continue;
                iterator.remove();
                this.c.d(l4);
            }
        }
    }
}

