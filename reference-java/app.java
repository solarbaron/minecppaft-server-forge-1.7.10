/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class app
extends aji {
    private final boolean a;

    public app(boolean bl2) {
        super(awt.H);
        this.a = bl2;
        this.a(i);
        this.c(0.5f);
        this.a(abt.d);
    }

    @Override
    public int b() {
        return 16;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = app.a(ahb2, n2, n3, n4, sv2);
        ahb2.a(n2, n3, n4, n5, 2);
        if (!ahb2.E) {
            this.e(ahb2, n2, n3, n4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb2.E) {
            this.e(ahb2, n2, n3, n4);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb2.E && ahb2.o(n2, n3, n4) == null) {
            this.e(ahb2, n2, n3, n4);
        }
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = app.b(n5);
        if (n6 == 7) {
            return;
        }
        boolean bl2 = this.a(ahb2, n2, n3, n4, n6);
        if (bl2 && !app.c(n5)) {
            if (app.h(ahb2, n2, n3, n4, n6)) {
                ahb2.c(n2, n3, n4, this, 0, n6);
            }
        } else if (!bl2 && app.c(n5)) {
            ahb2.a(n2, n3, n4, n6, 2);
            ahb2.c(n2, n3, n4, this, 1, n6);
        }
    }

    private boolean a(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (n5 != 0 && ahb2.f(n2, n3 - 1, n4, 0)) {
            return true;
        }
        if (n5 != 1 && ahb2.f(n2, n3 + 1, n4, 1)) {
            return true;
        }
        if (n5 != 2 && ahb2.f(n2, n3, n4 - 1, 2)) {
            return true;
        }
        if (n5 != 3 && ahb2.f(n2, n3, n4 + 1, 3)) {
            return true;
        }
        if (n5 != 5 && ahb2.f(n2 + 1, n3, n4, 5)) {
            return true;
        }
        if (n5 != 4 && ahb2.f(n2 - 1, n3, n4, 4)) {
            return true;
        }
        if (ahb2.f(n2, n3, n4, 0)) {
            return true;
        }
        if (ahb2.f(n2, n3 + 2, n4, 1)) {
            return true;
        }
        if (ahb2.f(n2, n3 + 1, n4 - 1, 2)) {
            return true;
        }
        if (ahb2.f(n2, n3 + 1, n4 + 1, 3)) {
            return true;
        }
        if (ahb2.f(n2 - 1, n3 + 1, n4, 4)) {
            return true;
        }
        return ahb2.f(n2 + 1, n3 + 1, n4, 5);
    }

    /*
     * Enabled force condition propagation
     * Lifted jumps to return sites
     */
    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        if (!ahb2.E) {
            boolean bl2 = this.a(ahb2, n2, n3, n4, n6);
            if (bl2 && n5 == 1) {
                ahb2.a(n2, n3, n4, n6 | 8, 2);
                return false;
            }
            if (!bl2 && n5 == 0) {
                return false;
            }
        }
        if (n5 == 0) {
            if (!this.i(ahb2, n2, n3, n4, n6)) return false;
            ahb2.a(n2, n3, n4, n6 | 8, 2);
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "tile.piston.out", 0.5f, ahb2.s.nextFloat() * 0.25f + 0.6f);
            return true;
        } else {
            if (n5 != 1) return true;
            aor aor2 = ahb2.o(n2 + q.b[n6], n3 + q.c[n6], n4 + q.d[n6]);
            if (aor2 instanceof aps) {
                ((aps)aor2).f();
            }
            ahb2.d(n2, n3, n4, ajn.M, n6, 3);
            ahb2.a(n2, n3, n4, apr.a(this, n6, n6, false, true));
            if (this.a) {
                aps aps2;
                aor aor3;
                int n7 = n2 + q.b[n6] * 2;
                int n8 = n3 + q.c[n6] * 2;
                int n9 = n4 + q.d[n6] * 2;
                aji aji2 = ahb2.a(n7, n8, n9);
                int n10 = ahb2.e(n7, n8, n9);
                boolean bl3 = false;
                if (aji2 == ajn.M && (aor3 = ahb2.o(n7, n8, n9)) instanceof aps && (aps2 = (aps)aor3).c() == n6 && aps2.b()) {
                    aps2.f();
                    aji2 = aps2.a();
                    n10 = aps2.p();
                    bl3 = true;
                }
                if (!bl3 && aji2.o() != awt.a && app.a(aji2, ahb2, n7, n8, n9, false) && (aji2.h() == 0 || aji2 == ajn.J || aji2 == ajn.F)) {
                    ahb2.d(n2 += q.b[n6], n3 += q.c[n6], n4 += q.d[n6], ajn.M, n10, 3);
                    ahb2.a(n2, n3, n4, apr.a(aji2, n10, n6, false, false));
                    ahb2.f(n7, n8, n9);
                } else if (!bl3) {
                    ahb2.f(n2 + q.b[n6], n3 + q.c[n6], n4 + q.d[n6]);
                }
            } else {
                ahb2.f(n2 + q.b[n6], n3 + q.c[n6], n4 + q.d[n6]);
            }
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "tile.piston.in", 0.5f, ahb2.s.nextFloat() * 0.15f + 0.6f);
        }
        return true;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        if (app.c(n5)) {
            float f2 = 0.25f;
            switch (app.b(n5)) {
                case 0: {
                    this.a(0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f);
                    break;
                }
                case 1: {
                    this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
                    break;
                }
                case 2: {
                    this.a(0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f);
                    break;
                }
                case 3: {
                    this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f);
                    break;
                }
                case 4: {
                    this.a(0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
                    break;
                }
                case 5: {
                    this.a(0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f);
                }
            }
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    @Override
    public void g() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public boolean d() {
        return false;
    }

    public static int b(int n2) {
        return n2 & 7;
    }

    public static boolean c(int n2) {
        return (n2 & 8) != 0;
    }

    public static int a(ahb ahb2, int n2, int n3, int n4, sv sv2) {
        int n5;
        if (qh.e((float)sv2.s - (float)n2) < 2.0f && qh.e((float)sv2.u - (float)n4) < 2.0f) {
            double d2 = sv2.t + 1.82 - (double)sv2.L;
            if (d2 - (double)n3 > 2.0) {
                return 1;
            }
            if ((double)n3 - d2 > 0.0) {
                return 0;
            }
        }
        if ((n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) == 0) {
            return 2;
        }
        if (n5 == 1) {
            return 5;
        }
        if (n5 == 2) {
            return 3;
        }
        if (n5 == 3) {
            return 4;
        }
        return 0;
    }

    private static boolean a(aji aji2, ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        if (aji2 == ajn.Z) {
            return false;
        }
        if (aji2 == ajn.J || aji2 == ajn.F) {
            if (app.c(ahb2.e(n2, n3, n4))) {
                return false;
            }
        } else {
            if (aji2.f(ahb2, n2, n3, n4) == -1.0f) {
                return false;
            }
            if (aji2.h() == 2) {
                return false;
            }
            if (aji2.h() == 1) {
                return bl2;
            }
        }
        return !(aji2 instanceof akw);
    }

    private static boolean h(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = n2 + q.b[n5];
        int n7 = n3 + q.c[n5];
        int n8 = n4 + q.d[n5];
        for (int i2 = 0; i2 < 13; ++i2) {
            if (n7 <= 0 || n7 >= 255) {
                return false;
            }
            aji aji2 = ahb2.a(n6, n7, n8);
            if (aji2.o() == awt.a) break;
            if (!app.a(aji2, ahb2, n6, n7, n8, true)) {
                return false;
            }
            if (aji2.h() == 1) break;
            if (i2 == 12) {
                return false;
            }
            n6 += q.b[n5];
            n7 += q.c[n5];
            n8 += q.d[n5];
        }
        return true;
    }

    private boolean i(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6;
        int n7;
        int n8;
        int n9;
        int n10 = n2 + q.b[n5];
        int n11 = n3 + q.c[n5];
        int n12 = n4 + q.d[n5];
        for (n9 = 0; n9 < 13; ++n9) {
            if (n11 <= 0 || n11 >= 255) {
                return false;
            }
            aji aji2 = ahb2.a(n10, n11, n12);
            if (aji2.o() == awt.a) break;
            if (!app.a(aji2, ahb2, n10, n11, n12, true)) {
                return false;
            }
            if (aji2.h() == 1) {
                aji2.b(ahb2, n10, n11, n12, ahb2.e(n10, n11, n12), 0);
                ahb2.f(n10, n11, n12);
                break;
            }
            if (n9 == 12) {
                return false;
            }
            n10 += q.b[n5];
            n11 += q.c[n5];
            n12 += q.d[n5];
        }
        n9 = n10;
        int n13 = n11;
        int n14 = n12;
        int n15 = 0;
        aji[] ajiArray = new aji[13];
        while (n10 != n2 || n11 != n3 || n12 != n4) {
            n8 = n10 - q.b[n5];
            n7 = n11 - q.c[n5];
            n6 = n12 - q.d[n5];
            aji aji3 = ahb2.a(n8, n7, n6);
            int n16 = ahb2.e(n8, n7, n6);
            if (aji3 == this && n8 == n2 && n7 == n3 && n6 == n4) {
                ahb2.d(n10, n11, n12, ajn.M, n5 | (this.a ? 8 : 0), 4);
                ahb2.a(n10, n11, n12, apr.a(ajn.K, n5 | (this.a ? 8 : 0), n5, true, false));
            } else {
                ahb2.d(n10, n11, n12, ajn.M, n16, 4);
                ahb2.a(n10, n11, n12, apr.a(aji3, n16, n5, true, false));
            }
            ajiArray[n15++] = aji3;
            n10 = n8;
            n11 = n7;
            n12 = n6;
        }
        n10 = n9;
        n11 = n13;
        n12 = n14;
        n15 = 0;
        while (n10 != n2 || n11 != n3 || n12 != n4) {
            n8 = n10 - q.b[n5];
            n7 = n11 - q.c[n5];
            n6 = n12 - q.d[n5];
            ahb2.d(n8, n7, n6, ajiArray[n15++]);
            n10 = n8;
            n11 = n7;
            n12 = n6;
        }
        return true;
    }
}

