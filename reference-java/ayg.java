/*
 * Decompiled with CFR 0.152.
 */
public class ayg {
    private ahl a;
    private ayd b = new ayd();
    private pz c = new pz();
    private aye[] d = new aye[32];
    private boolean e;
    private boolean f;
    private boolean g;
    private boolean h;

    public ayg(ahl ahl2, boolean bl2, boolean bl3, boolean bl4, boolean bl5) {
        this.a = ahl2;
        this.e = bl2;
        this.f = bl3;
        this.g = bl4;
        this.h = bl5;
    }

    public ayf a(sa sa2, sa sa3, float f2) {
        return this.a(sa2, sa3.s, sa3.C.b, sa3.u, f2);
    }

    public ayf a(sa sa2, int n2, int n3, int n4, float f2) {
        return this.a(sa2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, f2);
    }

    private ayf a(sa sa2, double d2, double d3, double d4, float f2) {
        Object object;
        this.b.a();
        this.c.c();
        boolean bl2 = this.g;
        int n2 = qh.c(sa2.C.b + 0.5);
        if (this.h && sa2.M()) {
            n2 = (int)sa2.C.b;
            object = this.a.a(qh.c(sa2.s), n2, qh.c(sa2.u));
            while (object == ajn.i || object == ajn.j) {
                object = this.a.a(qh.c(sa2.s), ++n2, qh.c(sa2.u));
            }
            bl2 = this.g;
            this.g = false;
        } else {
            n2 = qh.c(sa2.C.b + 0.5);
        }
        object = this.a(qh.c(sa2.C.a), n2, qh.c(sa2.C.c));
        aye aye2 = this.a(qh.c(d2 - (double)(sa2.M / 2.0f)), qh.c(d3), qh.c(d4 - (double)(sa2.M / 2.0f)));
        aye aye3 = new aye(qh.d(sa2.M + 1.0f), qh.d(sa2.N + 1.0f), qh.d(sa2.M + 1.0f));
        ayf ayf2 = this.a(sa2, (aye)object, aye2, aye3, f2);
        this.g = bl2;
        return ayf2;
    }

    private ayf a(sa sa2, aye aye2, aye aye3, aye aye4, float f2) {
        aye2.e = 0.0f;
        aye2.g = aye2.f = aye2.b(aye3);
        this.b.a();
        this.b.a(aye2);
        aye aye5 = aye2;
        while (!this.b.e()) {
            aye aye6 = this.b.c();
            if (aye6.equals(aye3)) {
                return this.a(aye2, aye3);
            }
            if (aye6.b(aye3) < aye5.b(aye3)) {
                aye5 = aye6;
            }
            aye6.i = true;
            int n2 = this.b(sa2, aye6, aye4, aye3, f2);
            for (int i2 = 0; i2 < n2; ++i2) {
                aye aye7 = this.d[i2];
                float f3 = aye6.e + aye6.b(aye7);
                if (aye7.a() && !(f3 < aye7.e)) continue;
                aye7.h = aye6;
                aye7.e = f3;
                aye7.f = aye7.b(aye3);
                if (aye7.a()) {
                    this.b.a(aye7, aye7.e + aye7.f);
                    continue;
                }
                aye7.g = aye7.e + aye7.f;
                this.b.a(aye7);
            }
        }
        if (aye5 == aye2) {
            return null;
        }
        return this.a(aye2, aye5);
    }

    private int b(sa sa2, aye aye2, aye aye3, aye aye4, float f2) {
        int n2 = 0;
        int n3 = 0;
        if (this.a(sa2, aye2.a, aye2.b + 1, aye2.c, aye3) == 1) {
            n3 = 1;
        }
        aye aye5 = this.a(sa2, aye2.a, aye2.b, aye2.c + 1, aye3, n3);
        aye aye6 = this.a(sa2, aye2.a - 1, aye2.b, aye2.c, aye3, n3);
        aye aye7 = this.a(sa2, aye2.a + 1, aye2.b, aye2.c, aye3, n3);
        aye aye8 = this.a(sa2, aye2.a, aye2.b, aye2.c - 1, aye3, n3);
        if (aye5 != null && !aye5.i && aye5.a(aye4) < f2) {
            this.d[n2++] = aye5;
        }
        if (aye6 != null && !aye6.i && aye6.a(aye4) < f2) {
            this.d[n2++] = aye6;
        }
        if (aye7 != null && !aye7.i && aye7.a(aye4) < f2) {
            this.d[n2++] = aye7;
        }
        if (aye8 != null && !aye8.i && aye8.a(aye4) < f2) {
            this.d[n2++] = aye8;
        }
        return n2;
    }

    private aye a(sa sa2, int n2, int n3, int n4, aye aye2, int n5) {
        aye aye3 = null;
        int n6 = this.a(sa2, n2, n3, n4, aye2);
        if (n6 == 2) {
            return this.a(n2, n3, n4);
        }
        if (n6 == 1) {
            aye3 = this.a(n2, n3, n4);
        }
        if (aye3 == null && n5 > 0 && n6 != -3 && n6 != -4 && this.a(sa2, n2, n3 + n5, n4, aye2) == 1) {
            aye3 = this.a(n2, n3 + n5, n4);
            n3 += n5;
        }
        if (aye3 != null) {
            int n7 = 0;
            int n8 = 0;
            while (n3 > 0) {
                n8 = this.a(sa2, n2, n3 - 1, n4, aye2);
                if (this.g && n8 == -1) {
                    return null;
                }
                if (n8 != 1) break;
                if (n7++ >= sa2.ax()) {
                    return null;
                }
                if (--n3 <= 0) continue;
                aye3 = this.a(n2, n3, n4);
            }
            if (n8 == -2) {
                return null;
            }
        }
        return aye3;
    }

    private final aye a(int n2, int n3, int n4) {
        int n5 = aye.a(n2, n3, n4);
        aye aye2 = (aye)this.c.a(n5);
        if (aye2 == null) {
            aye2 = new aye(n2, n3, n4);
            this.c.a(n5, aye2);
        }
        return aye2;
    }

    public int a(sa sa2, int n2, int n3, int n4, aye aye2) {
        return ayg.a(sa2, n2, n3, n4, aye2, this.g, this.f, this.e);
    }

    public static int a(sa sa2, int n2, int n3, int n4, aye aye2, boolean bl2, boolean bl3, boolean bl4) {
        boolean bl5 = false;
        for (int i2 = n2; i2 < n2 + aye2.a; ++i2) {
            for (int i3 = n3; i3 < n3 + aye2.b; ++i3) {
                for (int i4 = n4; i4 < n4 + aye2.c; ++i4) {
                    aji aji2 = sa2.o.a(i2, i3, i4);
                    if (aji2.o() == awt.a) continue;
                    if (aji2 == ajn.aT) {
                        bl5 = true;
                    } else if (aji2 == ajn.i || aji2 == ajn.j) {
                        if (bl2) {
                            return -1;
                        }
                        bl5 = true;
                    } else if (!bl4 && aji2 == ajn.ao) {
                        return 0;
                    }
                    int n5 = aji2.b();
                    if (sa2.o.a(i2, i3, i4).b() == 9) {
                        int n6;
                        int n7;
                        int n8 = qh.c(sa2.s);
                        if (sa2.o.a(n8, n7 = qh.c(sa2.t), n6 = qh.c(sa2.u)).b() == 9 || sa2.o.a(n8, n7 - 1, n6).b() == 9) continue;
                        return -3;
                    }
                    if (aji2.b((ahl)sa2.o, i2, i3, i4) || bl3 && aji2 == ajn.ao) continue;
                    if (n5 == 11 || aji2 == ajn.be || n5 == 32) {
                        return -3;
                    }
                    if (aji2 == ajn.aT) {
                        return -4;
                    }
                    awt awt2 = aji2.o();
                    if (awt2 == awt.i) {
                        if (sa2.P()) continue;
                        return -2;
                    }
                    return 0;
                }
            }
        }
        return bl5 ? 2 : 1;
    }

    private ayf a(aye aye2, aye aye3) {
        int n2 = 1;
        aye aye4 = aye3;
        while (aye4.h != null) {
            ++n2;
            aye4 = aye4.h;
        }
        aye[] ayeArray = new aye[n2];
        aye4 = aye3;
        ayeArray[--n2] = aye4;
        while (aye4.h != null) {
            aye4 = aye4.h;
            ayeArray[--n2] = aye4;
        }
        return new ayf(ayeArray);
    }
}

