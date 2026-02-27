/*
 * Decompiled with CFR 0.152.
 */
public class amw
extends akk {
    private boolean a;

    protected amw(boolean bl2) {
        super(awt.C);
        this.a(true);
        this.a = bl2;
        this.a(abt.b);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        if (ahb2.a(n2, n3 - 1, n4) == ajn.aE && ahb2.a(n2, n3 - 2, n4) == ajn.aE) {
            if (!ahb2.E) {
                ahb2.d(n2, n3, n4, amw.e(0), 0, 2);
                ahb2.d(n2, n3 - 1, n4, amw.e(0), 0, 2);
                ahb2.d(n2, n3 - 2, n4, amw.e(0), 0, 2);
                wr wr2 = new wr(ahb2);
                wr2.b((double)n2 + 0.5, (double)n3 - 1.95, (double)n4 + 0.5, 0.0f, 0.0f);
                ahb2.d(wr2);
                ahb2.c(n2, n3, n4, amw.e(0));
                ahb2.c(n2, n3 - 1, n4, amw.e(0));
                ahb2.c(n2, n3 - 2, n4, amw.e(0));
            }
            for (int i2 = 0; i2 < 120; ++i2) {
                ahb2.a("snowshovel", (double)n2 + ahb2.s.nextDouble(), (double)(n3 - 2) + ahb2.s.nextDouble() * 2.5, (double)n4 + ahb2.s.nextDouble(), 0.0, 0.0, 0.0);
            }
        } else if (ahb2.a(n2, n3 - 1, n4) == ajn.S && ahb2.a(n2, n3 - 2, n4) == ajn.S) {
            boolean bl2;
            boolean bl3 = ahb2.a(n2 - 1, n3 - 1, n4) == ajn.S && ahb2.a(n2 + 1, n3 - 1, n4) == ajn.S;
            boolean bl4 = bl2 = ahb2.a(n2, n3 - 1, n4 - 1) == ajn.S && ahb2.a(n2, n3 - 1, n4 + 1) == ajn.S;
            if (bl3 || bl2) {
                ahb2.d(n2, n3, n4, amw.e(0), 0, 2);
                ahb2.d(n2, n3 - 1, n4, amw.e(0), 0, 2);
                ahb2.d(n2, n3 - 2, n4, amw.e(0), 0, 2);
                if (bl3) {
                    ahb2.d(n2 - 1, n3 - 1, n4, amw.e(0), 0, 2);
                    ahb2.d(n2 + 1, n3 - 1, n4, amw.e(0), 0, 2);
                } else {
                    ahb2.d(n2, n3 - 1, n4 - 1, amw.e(0), 0, 2);
                    ahb2.d(n2, n3 - 1, n4 + 1, amw.e(0), 0, 2);
                }
                wt wt2 = new wt(ahb2);
                wt2.i(true);
                wt2.b((double)n2 + 0.5, (double)n3 - 1.95, (double)n4 + 0.5, 0.0f, 0.0f);
                ahb2.d(wt2);
                for (int i3 = 0; i3 < 120; ++i3) {
                    ahb2.a("snowballpoof", (double)n2 + ahb2.s.nextDouble(), (double)(n3 - 2) + ahb2.s.nextDouble() * 3.9, (double)n4 + ahb2.s.nextDouble(), 0.0, 0.0, 0.0);
                }
                ahb2.c(n2, n3, n4, amw.e(0));
                ahb2.c(n2, n3 - 1, n4, amw.e(0));
                ahb2.c(n2, n3 - 2, n4, amw.e(0));
                if (bl3) {
                    ahb2.c(n2 - 1, n3 - 1, n4, amw.e(0));
                    ahb2.c(n2 + 1, n3 - 1, n4, amw.e(0));
                } else {
                    ahb2.c(n2, n3 - 1, n4 - 1, amw.e(0));
                    ahb2.c(n2, n3 - 1, n4 + 1, amw.e(0));
                }
            }
        }
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.a((int)n2, (int)n3, (int)n4).J.j() && ahb.a(ahb2, n2, n3 - 1, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 2.5) & 3;
        ahb2.a(n2, n3, n4, n5, 2);
    }
}

