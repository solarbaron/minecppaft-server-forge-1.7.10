/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class asy
extends avk {
    private boolean a;
    private boolean b;
    private boolean c;
    private int d;

    public asy() {
    }

    @Override
    protected void a(dh dh2) {
        dh2.a("hr", this.a);
        dh2.a("sc", this.b);
        dh2.a("hps", this.c);
        dh2.a("Num", this.d);
    }

    @Override
    protected void b(dh dh2) {
        this.a = dh2.n("hr");
        this.b = dh2.n("sc");
        this.c = dh2.n("hps");
        this.d = dh2.f("Num");
    }

    public asy(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
        this.a = random.nextInt(3) == 0;
        this.b = !this.a && random.nextInt(23) == 0;
        this.d = this.g == 2 || this.g == 0 ? asv2.d() / 5 : asv2.b() / 5;
    }

    public static asv a(List list, Random random, int n2, int n3, int n4, int n5) {
        int n6;
        asv asv2 = new asv(n2, n3, n4, n2, n3 + 2, n4);
        for (n6 = random.nextInt(3) + 2; n6 > 0; --n6) {
            int n7 = n6 * 5;
            switch (n5) {
                case 2: {
                    asv2.d = n2 + 2;
                    asv2.c = n4 - (n7 - 1);
                    break;
                }
                case 0: {
                    asv2.d = n2 + 2;
                    asv2.f = n4 + (n7 - 1);
                    break;
                }
                case 1: {
                    asv2.a = n2 - (n7 - 1);
                    asv2.f = n4 + 2;
                    break;
                }
                case 3: {
                    asv2.d = n2 + (n7 - 1);
                    asv2.f = n4 + 2;
                }
            }
            if (avk.a(list, asv2) == null) break;
        }
        if (n6 > 0) {
            return asv2;
        }
        return null;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        block24: {
            int n2 = this.d();
            int n3 = random.nextInt(4);
            switch (this.g) {
                case 2: {
                    if (n3 <= 1) {
                        asx.a(avk2, list, random, this.f.a, this.f.b - 1 + random.nextInt(3), this.f.c - 1, this.g, n2);
                        break;
                    }
                    if (n3 == 2) {
                        asx.a(avk2, list, random, this.f.a - 1, this.f.b - 1 + random.nextInt(3), this.f.c, 1, n2);
                        break;
                    }
                    asx.a(avk2, list, random, this.f.d + 1, this.f.b - 1 + random.nextInt(3), this.f.c, 3, n2);
                    break;
                }
                case 0: {
                    if (n3 <= 1) {
                        asx.a(avk2, list, random, this.f.a, this.f.b - 1 + random.nextInt(3), this.f.f + 1, this.g, n2);
                        break;
                    }
                    if (n3 == 2) {
                        asx.a(avk2, list, random, this.f.a - 1, this.f.b - 1 + random.nextInt(3), this.f.f - 3, 1, n2);
                        break;
                    }
                    asx.a(avk2, list, random, this.f.d + 1, this.f.b - 1 + random.nextInt(3), this.f.f - 3, 3, n2);
                    break;
                }
                case 1: {
                    if (n3 <= 1) {
                        asx.a(avk2, list, random, this.f.a - 1, this.f.b - 1 + random.nextInt(3), this.f.c, this.g, n2);
                        break;
                    }
                    if (n3 == 2) {
                        asx.a(avk2, list, random, this.f.a, this.f.b - 1 + random.nextInt(3), this.f.c - 1, 2, n2);
                        break;
                    }
                    asx.a(avk2, list, random, this.f.a, this.f.b - 1 + random.nextInt(3), this.f.f + 1, 0, n2);
                    break;
                }
                case 3: {
                    if (n3 <= 1) {
                        asx.a(avk2, list, random, this.f.d + 1, this.f.b - 1 + random.nextInt(3), this.f.c, this.g, n2);
                        break;
                    }
                    if (n3 == 2) {
                        asx.a(avk2, list, random, this.f.d - 3, this.f.b - 1 + random.nextInt(3), this.f.c - 1, 2, n2);
                        break;
                    }
                    asx.a(avk2, list, random, this.f.d - 3, this.f.b - 1 + random.nextInt(3), this.f.f + 1, 0, n2);
                }
            }
            if (n2 >= 8) break block24;
            if (this.g == 2 || this.g == 0) {
                int n4 = this.f.c + 3;
                while (n4 + 3 <= this.f.f) {
                    int n5 = random.nextInt(5);
                    if (n5 == 0) {
                        asx.a(avk2, list, random, this.f.a - 1, this.f.b, n4, 1, n2 + 1);
                    } else if (n5 == 1) {
                        asx.a(avk2, list, random, this.f.d + 1, this.f.b, n4, 3, n2 + 1);
                    }
                    n4 += 5;
                }
            } else {
                int n6 = this.f.a + 3;
                while (n6 + 3 <= this.f.d) {
                    int n7 = random.nextInt(5);
                    if (n7 == 0) {
                        asx.a(avk2, list, random, n6, this.f.b, this.f.c - 1, 2, n2 + 1);
                    } else if (n7 == 1) {
                        asx.a(avk2, list, random, n6, this.f.b, this.f.f + 1, 0, n2 + 1);
                    }
                    n6 += 5;
                }
            }
        }
    }

    @Override
    protected boolean a(ahb ahb2, asv asv2, Random random, int n2, int n3, int n4, qx[] qxArray, int n5) {
        int n6;
        int n7;
        int n8 = this.a(n2, n4);
        if (asv2.b(n8, n7 = this.a(n3), n6 = this.b(n2, n4)) && ahb2.a(n8, n7, n6).o() == awt.a) {
            int n9 = random.nextBoolean() ? 1 : 0;
            ahb2.d(n8, n7, n6, ajn.aq, this.a(ajn.aq, n9), 2);
            xm xm2 = new xm(ahb2, (float)n8 + 0.5f, (float)n7 + 0.5f, (float)n6 + 0.5f);
            qx.a(random, qxArray, xm2, n5);
            ahb2.d(xm2);
            return true;
        }
        return false;
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        int n4;
        int n5;
        if (this.a(ahb2, asv2)) {
            return false;
        }
        boolean bl2 = false;
        int n6 = 2;
        boolean bl3 = false;
        int n7 = 2;
        int n8 = this.d * 5 - 1;
        this.a(ahb2, asv2, 0, 0, 0, 2, 1, n8, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, random, 0.8f, 0, 2, 0, 2, 2, n8, ajn.a, ajn.a, false);
        if (this.b) {
            this.a(ahb2, asv2, random, 0.6f, 0, 0, 0, 2, 1, n8, ajn.G, ajn.a, false);
        }
        for (n5 = 0; n5 < this.d; ++n5) {
            n4 = 2 + n5 * 5;
            this.a(ahb2, asv2, 0, 0, n4, 0, 1, n4, ajn.aJ, ajn.a, false);
            this.a(ahb2, asv2, 2, 0, n4, 2, 1, n4, ajn.aJ, ajn.a, false);
            if (random.nextInt(4) == 0) {
                this.a(ahb2, asv2, 0, 2, n4, 0, 2, n4, ajn.f, ajn.a, false);
                this.a(ahb2, asv2, 2, 2, n4, 2, 2, n4, ajn.f, ajn.a, false);
            } else {
                this.a(ahb2, asv2, 0, 2, n4, 2, 2, n4, ajn.f, ajn.a, false);
            }
            this.a(ahb2, asv2, random, 0.1f, 0, 2, n4 - 1, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.1f, 2, 2, n4 - 1, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.1f, 0, 2, n4 + 1, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.1f, 2, 2, n4 + 1, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.05f, 0, 2, n4 - 2, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.05f, 2, 2, n4 - 2, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.05f, 0, 2, n4 + 2, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.05f, 2, 2, n4 + 2, ajn.G, 0);
            this.a(ahb2, asv2, random, 0.05f, 1, 2, n4 - 1, ajn.aa, 0);
            this.a(ahb2, asv2, random, 0.05f, 1, 2, n4 + 1, ajn.aa, 0);
            if (random.nextInt(100) == 0) {
                this.a(ahb2, asv2, random, 2, 0, n4 - 1, qx.a(asx.b(), ade.bR.b(random)), 3 + random.nextInt(4));
            }
            if (random.nextInt(100) == 0) {
                this.a(ahb2, asv2, random, 0, 0, n4 + 1, qx.a(asx.b(), ade.bR.b(random)), 3 + random.nextInt(4));
            }
            if (!this.b || this.c) continue;
            n3 = this.a(0);
            int n9 = n4 - 1 + random.nextInt(3);
            n2 = this.a(1, n9);
            if (!asv2.b(n2, n3, n9 = this.b(1, n9))) continue;
            this.c = true;
            ahb2.d(n2, n3, n9, ajn.ac, 0, 2);
            apj apj2 = (apj)ahb2.o(n2, n3, n9);
            if (apj2 == null) continue;
            apj2.a().a("CaveSpider");
        }
        for (n5 = 0; n5 <= 2; ++n5) {
            for (n4 = 0; n4 <= n8; ++n4) {
                n3 = -1;
                aji aji2 = this.a(ahb2, n5, n3, n4, asv2);
                if (aji2.o() != awt.a) continue;
                n2 = -1;
                this.a(ahb2, ajn.f, 0, n5, n2, n4, asv2);
            }
        }
        if (this.a) {
            for (n5 = 0; n5 <= n8; ++n5) {
                aji aji3 = this.a(ahb2, 1, -1, n5, asv2);
                if (aji3.o() == awt.a || !aji3.j()) continue;
                this.a(ahb2, asv2, random, 0.7f, 1, 0, n5, ajn.aq, this.a(ajn.aq, 0));
            }
        }
        return true;
    }
}

