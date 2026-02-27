/*
 * Decompiled with CFR 0.152.
 */
public class aoe
extends aji {
    protected aoe(awt awt2) {
        super(awt2);
        float f2 = 0.5f;
        float f3 = 1.0f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f3, 0.5f + f2);
        this.a(abt.d);
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
    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        return !aoe.d(ahl2.e(n2, n3, n4));
    }

    @Override
    public int b() {
        return 0;
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.b(ahl2.e(n2, n3, n4));
    }

    @Override
    public void g() {
        float f2 = 0.1875f;
        this.a(0.0f, 0.5f - f2 / 2.0f, 0.0f, 1.0f, 0.5f + f2 / 2.0f, 1.0f);
    }

    public void b(int n2) {
        float f2 = 0.1875f;
        if ((n2 & 8) != 0) {
            this.a(0.0f, 1.0f - f2, 0.0f, 1.0f, 1.0f, 1.0f);
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, f2, 1.0f);
        }
        if (aoe.d(n2)) {
            if ((n2 & 3) == 0) {
                this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
            }
            if ((n2 & 3) == 1) {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
            }
            if ((n2 & 3) == 2) {
                this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            }
            if ((n2 & 3) == 3) {
                this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (this.J == awt.f) {
            return true;
        }
        int n6 = ahb2.e(n2, n3, n4);
        ahb2.a(n2, n3, n4, n6 ^ 4, 2);
        ahb2.a(yz2, 1003, n2, n3, n4, 0);
        return true;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        boolean bl3;
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl4 = bl3 = (n5 & 4) > 0;
        if (bl3 == bl2) {
            return;
        }
        ahb2.a(n2, n3, n4, n5 ^ 4, 2);
        ahb2.a(null, 1003, n2, n3, n4, 0);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        boolean bl2;
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = n2;
        int n7 = n4;
        if ((n5 & 3) == 0) {
            ++n7;
        }
        if ((n5 & 3) == 1) {
            --n7;
        }
        if ((n5 & 3) == 2) {
            ++n6;
        }
        if ((n5 & 3) == 3) {
            --n6;
        }
        if (!aoe.a(ahb2.a(n6, n3, n7))) {
            ahb2.f(n2, n3, n4);
            this.b(ahb2, n2, n3, n4, n5, 0);
        }
        if ((bl2 = ahb2.v(n2, n3, n4)) || aji2.f()) {
            this.a(ahb2, n2, n3, n4, bl2);
        }
    }

    @Override
    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4, azw2, azw3);
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = 0;
        if (n5 == 2) {
            n7 = 0;
        }
        if (n5 == 3) {
            n7 = 1;
        }
        if (n5 == 4) {
            n7 = 2;
        }
        if (n5 == 5) {
            n7 = 3;
        }
        if (n5 != 1 && n5 != 0 && f3 > 0.5f) {
            n7 |= 8;
        }
        return n7;
    }

    @Override
    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (n5 == 0) {
            return false;
        }
        if (n5 == 1) {
            return false;
        }
        if (n5 == 2) {
            ++n4;
        }
        if (n5 == 3) {
            --n4;
        }
        if (n5 == 4) {
            ++n2;
        }
        if (n5 == 5) {
            --n2;
        }
        return aoe.a(ahb2.a(n2, n3, n4));
    }

    public static boolean d(int n2) {
        return (n2 & 4) != 0;
    }

    private static boolean a(aji aji2) {
        return aji2.J.k() && aji2.d() || aji2 == ajn.aN || aji2 instanceof alj || aji2 instanceof ans;
    }
}

