/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class alt
extends aod {
    int[] a;
    protected rf[][] M = new rf[2][];

    public alt() {
        super(awt.j, false);
        this.a(true);
        this.a(abt.c);
        this.c(0.2f);
        this.g(1);
        this.a(h);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        int n6 = 1;
        int n7 = n6 + 1;
        if (ahb2.b(n2 - n7, n3 - n7, n4 - n7, n2 + n7, n3 + n7, n4 + n7)) {
            for (int i2 = -n6; i2 <= n6; ++i2) {
                for (int i3 = -n6; i3 <= n6; ++i3) {
                    for (int i4 = -n6; i4 <= n6; ++i4) {
                        if (ahb2.a(n2 + i2, n3 + i3, n4 + i4).o() != awt.j) continue;
                        int n8 = ahb2.e(n2 + i2, n3 + i3, n4 + i4);
                        ahb2.a(n2 + i2, n3 + i3, n4 + i4, n8 | 8, 4);
                    }
                }
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) != 0 && (n5 & 4) == 0) {
            int n6;
            int n7 = 4;
            int n8 = n7 + 1;
            int n9 = 32;
            int n10 = n9 * n9;
            int n11 = n9 / 2;
            if (this.a == null) {
                this.a = new int[n9 * n9 * n9];
            }
            if (ahb2.b(n2 - n8, n3 - n8, n4 - n8, n2 + n8, n3 + n8, n4 + n8)) {
                int n12;
                int n13;
                for (n6 = -n7; n6 <= n7; ++n6) {
                    for (n13 = -n7; n13 <= n7; ++n13) {
                        for (n12 = -n7; n12 <= n7; ++n12) {
                            aji aji2 = ahb2.a(n2 + n6, n3 + n13, n4 + n12);
                            this.a[(n6 + n11) * n10 + (n13 + n11) * n9 + (n12 + n11)] = aji2 == ajn.r || aji2 == ajn.s ? 0 : (aji2.o() == awt.j ? -2 : -1);
                        }
                    }
                }
                for (n6 = 1; n6 <= 4; ++n6) {
                    for (n13 = -n7; n13 <= n7; ++n13) {
                        for (n12 = -n7; n12 <= n7; ++n12) {
                            for (int i2 = -n7; i2 <= n7; ++i2) {
                                if (this.a[(n13 + n11) * n10 + (n12 + n11) * n9 + (i2 + n11)] != n6 - 1) continue;
                                if (this.a[(n13 + n11 - 1) * n10 + (n12 + n11) * n9 + (i2 + n11)] == -2) {
                                    this.a[(n13 + n11 - 1) * n10 + (n12 + n11) * n9 + (i2 + n11)] = n6;
                                }
                                if (this.a[(n13 + n11 + 1) * n10 + (n12 + n11) * n9 + (i2 + n11)] == -2) {
                                    this.a[(n13 + n11 + 1) * n10 + (n12 + n11) * n9 + (i2 + n11)] = n6;
                                }
                                if (this.a[(n13 + n11) * n10 + (n12 + n11 - 1) * n9 + (i2 + n11)] == -2) {
                                    this.a[(n13 + n11) * n10 + (n12 + n11 - 1) * n9 + (i2 + n11)] = n6;
                                }
                                if (this.a[(n13 + n11) * n10 + (n12 + n11 + 1) * n9 + (i2 + n11)] == -2) {
                                    this.a[(n13 + n11) * n10 + (n12 + n11 + 1) * n9 + (i2 + n11)] = n6;
                                }
                                if (this.a[(n13 + n11) * n10 + (n12 + n11) * n9 + (i2 + n11 - 1)] == -2) {
                                    this.a[(n13 + n11) * n10 + (n12 + n11) * n9 + (i2 + n11 - 1)] = n6;
                                }
                                if (this.a[(n13 + n11) * n10 + (n12 + n11) * n9 + (i2 + n11 + 1)] != -2) continue;
                                this.a[(n13 + n11) * n10 + (n12 + n11) * n9 + (i2 + n11 + 1)] = n6;
                            }
                        }
                    }
                }
            }
            if ((n6 = this.a[n11 * n10 + n11 * n9 + n11]) >= 0) {
                ahb2.a(n2, n3, n4, n5 & 0xFFFFFFF7, 4);
            } else {
                this.e(ahb2, n2, n3, n4);
            }
        }
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
        ahb2.f(n2, n3, n4);
    }

    @Override
    public int a(Random random) {
        return random.nextInt(20) == 0 ? 1 : 0;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.g);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (!ahb2.E) {
            int n7 = this.b(n5);
            if (n6 > 0 && (n7 -= 2 << n6) < 10) {
                n7 = 10;
            }
            if (ahb2.s.nextInt(n7) == 0) {
                adb adb2 = this.a(n5, ahb2.s, n6);
                this.a(ahb2, n2, n3, n4, new add(adb2, 1, this.a(n5)));
            }
            n7 = 200;
            if (n6 > 0 && (n7 -= 10 << n6) < 40) {
                n7 = 40;
            }
            this.c(ahb2, n2, n3, n4, n5, n7);
        }
    }

    protected void c(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
    }

    protected int b(int n2) {
        return 20;
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E && yz2.bF() != null && yz2.bF().b() == ade.aZ) {
            yz2.a(pp.C[aji.b(this)], 1);
            this.a(ahb2, n2, n3, n4, new add(adb.a(this), 1, n5 & 3));
        } else {
            super.a(ahb2, yz2, n2, n3, n4, n5);
        }
    }

    @Override
    public int a(int n2) {
        return n2 & 3;
    }

    @Override
    public boolean c() {
        return !this.P;
    }

    @Override
    protected add j(int n2) {
        return new add(adb.a(this), 1, n2 & 3);
    }

    public abstract String[] e();
}

