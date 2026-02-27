/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anl
extends ajc {
    protected anl() {
        super(awt.q);
        this.a(0.25f, 0.0f, 0.25f, 0.75f, 0.5f, 0.75f);
    }

    @Override
    public int b() {
        return -1;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4) & 7;
        switch (n5) {
            default: {
                this.a(0.25f, 0.0f, 0.25f, 0.75f, 0.5f, 0.75f);
                break;
            }
            case 2: {
                this.a(0.25f, 0.25f, 0.5f, 0.75f, 0.75f, 1.0f);
                break;
            }
            case 3: {
                this.a(0.25f, 0.25f, 0.0f, 0.75f, 0.75f, 0.5f);
                break;
            }
            case 4: {
                this.a(0.5f, 0.25f, 0.25f, 1.0f, 0.75f, 0.75f);
                break;
            }
            case 5: {
                this.a(0.0f, 0.25f, 0.25f, 0.5f, 0.75f, 0.75f);
            }
        }
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 2.5) & 3;
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apn();
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null && aor2 instanceof apn) {
            return ((apn)aor2).b();
        }
        return super.k(ahb2, n2, n3, n4);
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        if (yz2.bE.d) {
            ahb2.a(n2, n3, n4, n5 |= 8, 4);
        }
        super.a(ahb2, n2, n3, n4, n5, yz2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        if (ahb2.E) {
            return;
        }
        if ((n5 & 8) == 0) {
            add add2 = new add(ade.bL, 1, this.k(ahb2, n2, n3, n4));
            apn apn2 = (apn)ahb2.o(n2, n3, n4);
            if (apn2.b() == 3 && apn2.a() != null) {
                add2.d(new dh());
                dh dh2 = new dh();
                dv.a(dh2, apn2.a());
                add2.q().a("SkullOwner", dh2);
            }
            this.a(ahb2, n2, n3, n4, add2);
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.bL;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, apn apn2) {
        if (apn2.b() == 1 && n3 >= 2 && ahb2.r != rd.a && !ahb2.E) {
            int n5;
            for (n5 = -2; n5 <= 0; ++n5) {
                if (ahb2.a(n2, n3 - 1, n4 + n5) != ajn.aM || ahb2.a(n2, n3 - 1, n4 + n5 + 1) != ajn.aM || ahb2.a(n2, n3 - 2, n4 + n5 + 1) != ajn.aM || ahb2.a(n2, n3 - 1, n4 + n5 + 2) != ajn.aM || !this.a(ahb2, n2, n3, n4 + n5, 1) || !this.a(ahb2, n2, n3, n4 + n5 + 1, 1) || !this.a(ahb2, n2, n3, n4 + n5 + 2, 1)) continue;
                ahb2.a(n2, n3, n4 + n5, 8, 2);
                ahb2.a(n2, n3, n4 + n5 + 1, 8, 2);
                ahb2.a(n2, n3, n4 + n5 + 2, 8, 2);
                ahb2.d(n2, n3, n4 + n5, anl.e(0), 0, 2);
                ahb2.d(n2, n3, n4 + n5 + 1, anl.e(0), 0, 2);
                ahb2.d(n2, n3, n4 + n5 + 2, anl.e(0), 0, 2);
                ahb2.d(n2, n3 - 1, n4 + n5, anl.e(0), 0, 2);
                ahb2.d(n2, n3 - 1, n4 + n5 + 1, anl.e(0), 0, 2);
                ahb2.d(n2, n3 - 1, n4 + n5 + 2, anl.e(0), 0, 2);
                ahb2.d(n2, n3 - 2, n4 + n5 + 1, anl.e(0), 0, 2);
                if (!ahb2.E) {
                    xc xc2 = new xc(ahb2);
                    xc2.b((double)n2 + 0.5, (double)n3 - 1.45, (double)(n4 + n5) + 1.5, 90.0f, 0.0f);
                    xc2.aM = 90.0f;
                    xc2.bZ();
                    if (!ahb2.E) {
                        for (yz yz2 : ahb2.a(yz.class, xc2.C.b(50.0, 50.0, 50.0))) {
                            yz2.a(pc.I);
                        }
                    }
                    ahb2.d(xc2);
                }
                for (int i2 = 0; i2 < 120; ++i2) {
                    ahb2.a("snowballpoof", (double)n2 + ahb2.s.nextDouble(), (double)(n3 - 2) + ahb2.s.nextDouble() * 3.9, (double)(n4 + n5 + 1) + ahb2.s.nextDouble(), 0.0, 0.0, 0.0);
                }
                ahb2.c(n2, n3, n4 + n5, anl.e(0));
                ahb2.c(n2, n3, n4 + n5 + 1, anl.e(0));
                ahb2.c(n2, n3, n4 + n5 + 2, anl.e(0));
                ahb2.c(n2, n3 - 1, n4 + n5, anl.e(0));
                ahb2.c(n2, n3 - 1, n4 + n5 + 1, anl.e(0));
                ahb2.c(n2, n3 - 1, n4 + n5 + 2, anl.e(0));
                ahb2.c(n2, n3 - 2, n4 + n5 + 1, anl.e(0));
                return;
            }
            for (n5 = -2; n5 <= 0; ++n5) {
                if (ahb2.a(n2 + n5, n3 - 1, n4) != ajn.aM || ahb2.a(n2 + n5 + 1, n3 - 1, n4) != ajn.aM || ahb2.a(n2 + n5 + 1, n3 - 2, n4) != ajn.aM || ahb2.a(n2 + n5 + 2, n3 - 1, n4) != ajn.aM || !this.a(ahb2, n2 + n5, n3, n4, 1) || !this.a(ahb2, n2 + n5 + 1, n3, n4, 1) || !this.a(ahb2, n2 + n5 + 2, n3, n4, 1)) continue;
                ahb2.a(n2 + n5, n3, n4, 8, 2);
                ahb2.a(n2 + n5 + 1, n3, n4, 8, 2);
                ahb2.a(n2 + n5 + 2, n3, n4, 8, 2);
                ahb2.d(n2 + n5, n3, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5 + 1, n3, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5 + 2, n3, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5, n3 - 1, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5 + 1, n3 - 1, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5 + 2, n3 - 1, n4, anl.e(0), 0, 2);
                ahb2.d(n2 + n5 + 1, n3 - 2, n4, anl.e(0), 0, 2);
                if (!ahb2.E) {
                    xc xc3 = new xc(ahb2);
                    xc3.b((double)(n2 + n5) + 1.5, (double)n3 - 1.45, (double)n4 + 0.5, 0.0f, 0.0f);
                    xc3.bZ();
                    if (!ahb2.E) {
                        for (yz yz3 : ahb2.a(yz.class, xc3.C.b(50.0, 50.0, 50.0))) {
                            yz3.a(pc.I);
                        }
                    }
                    ahb2.d(xc3);
                }
                for (int i3 = 0; i3 < 120; ++i3) {
                    ahb2.a("snowballpoof", (double)(n2 + n5 + 1) + ahb2.s.nextDouble(), (double)(n3 - 2) + ahb2.s.nextDouble() * 3.9, (double)n4 + ahb2.s.nextDouble(), 0.0, 0.0, 0.0);
                }
                ahb2.c(n2 + n5, n3, n4, anl.e(0));
                ahb2.c(n2 + n5 + 1, n3, n4, anl.e(0));
                ahb2.c(n2 + n5 + 2, n3, n4, anl.e(0));
                ahb2.c(n2 + n5, n3 - 1, n4, anl.e(0));
                ahb2.c(n2 + n5 + 1, n3 - 1, n4, anl.e(0));
                ahb2.c(n2 + n5 + 2, n3 - 1, n4, anl.e(0));
                ahb2.c(n2 + n5 + 1, n3 - 2, n4, anl.e(0));
                return;
            }
        }
    }

    private boolean a(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (ahb2.a(n2, n3, n4) != this) {
            return false;
        }
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 == null || !(aor2 instanceof apn)) {
            return false;
        }
        return ((apn)aor2).b() == n5;
    }
}

