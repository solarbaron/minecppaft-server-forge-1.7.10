/*
 * Decompiled with CFR 0.152.
 */
public class alv
extends aji {
    protected alv() {
        super(awt.q);
        this.a(abt.d);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
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
    public int b() {
        return 12;
    }

    @Override
    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (n5 == 0 && ahb2.a(n2, n3 + 1, n4).r()) {
            return true;
        }
        if (n5 == 1 && ahb.a(ahb2, n2, n3 - 1, n4)) {
            return true;
        }
        if (n5 == 2 && ahb2.a(n2, n3, n4 + 1).r()) {
            return true;
        }
        if (n5 == 3 && ahb2.a(n2, n3, n4 - 1).r()) {
            return true;
        }
        if (n5 == 4 && ahb2.a(n2 + 1, n3, n4).r()) {
            return true;
        }
        return n5 == 5 && ahb2.a(n2 - 1, n3, n4).r();
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2 - 1, n3, n4).r()) {
            return true;
        }
        if (ahb2.a(n2 + 1, n3, n4).r()) {
            return true;
        }
        if (ahb2.a(n2, n3, n4 - 1).r()) {
            return true;
        }
        if (ahb2.a(n2, n3, n4 + 1).r()) {
            return true;
        }
        if (ahb.a(ahb2, n2, n3 - 1, n4)) {
            return true;
        }
        return ahb2.a(n2, n3 + 1, n4).r();
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = n6;
        int n8 = n7 & 8;
        n7 &= 7;
        n7 = -1;
        if (n5 == 0 && ahb2.a(n2, n3 + 1, n4).r()) {
            n7 = 0;
        }
        if (n5 == 1 && ahb.a(ahb2, n2, n3 - 1, n4)) {
            n7 = 5;
        }
        if (n5 == 2 && ahb2.a(n2, n3, n4 + 1).r()) {
            n7 = 4;
        }
        if (n5 == 3 && ahb2.a(n2, n3, n4 - 1).r()) {
            n7 = 3;
        }
        if (n5 == 4 && ahb2.a(n2 + 1, n3, n4).r()) {
            n7 = 2;
        }
        if (n5 == 5 && ahb2.a(n2 - 1, n3, n4).r()) {
            n7 = 1;
        }
        return n7 + n8;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = n5 & 7;
        int n7 = n5 & 8;
        if (n6 == alv.b(1)) {
            if ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 1) == 0) {
                ahb2.a(n2, n3, n4, 5 | n7, 2);
            } else {
                ahb2.a(n2, n3, n4, 6 | n7, 2);
            }
        } else if (n6 == alv.b(0)) {
            if ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 1) == 0) {
                ahb2.a(n2, n3, n4, 7 | n7, 2);
            } else {
                ahb2.a(n2, n3, n4, 0 | n7, 2);
            }
        }
    }

    public static int b(int n2) {
        switch (n2) {
            case 0: {
                return 0;
            }
            case 1: {
                return 5;
            }
            case 2: {
                return 4;
            }
            case 3: {
                return 3;
            }
            case 4: {
                return 2;
            }
            case 5: {
                return 1;
            }
        }
        return -1;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (this.e(ahb2, n2, n3, n4)) {
            int n5 = ahb2.e(n2, n3, n4) & 7;
            boolean bl2 = false;
            if (!ahb2.a(n2 - 1, n3, n4).r() && n5 == 1) {
                bl2 = true;
            }
            if (!ahb2.a(n2 + 1, n3, n4).r() && n5 == 2) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3, n4 - 1).r() && n5 == 3) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3, n4 + 1).r() && n5 == 4) {
                bl2 = true;
            }
            if (!ahb.a(ahb2, n2, n3 - 1, n4) && n5 == 5) {
                bl2 = true;
            }
            if (!ahb.a(ahb2, n2, n3 - 1, n4) && n5 == 6) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3 + 1, n4).r() && n5 == 0) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3 + 1, n4).r() && n5 == 7) {
                bl2 = true;
            }
            if (bl2) {
                this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
                ahb2.f(n2, n3, n4);
            }
        }
    }

    private boolean e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.c(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            return false;
        }
        return true;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4) & 7;
        float f2 = 0.1875f;
        if (n5 == 1) {
            this.a(0.0f, 0.2f, 0.5f - f2, f2 * 2.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 2) {
            this.a(1.0f - f2 * 2.0f, 0.2f, 0.5f - f2, 1.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 3) {
            this.a(0.5f - f2, 0.2f, 0.0f, 0.5f + f2, 0.8f, f2 * 2.0f);
        } else if (n5 == 4) {
            this.a(0.5f - f2, 0.2f, 1.0f - f2 * 2.0f, 0.5f + f2, 0.8f, 1.0f);
        } else if (n5 == 5 || n5 == 6) {
            f2 = 0.25f;
            this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.6f, 0.5f + f2);
        } else if (n5 == 0 || n5 == 7) {
            f2 = 0.25f;
            this.a(0.5f - f2, 0.4f, 0.5f - f2, 0.5f + f2, 1.0f, 0.5f + f2);
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = n6 & 7;
        int n8 = 8 - (n6 & 8);
        ahb2.a(n2, n3, n4, n7 + n8, 3);
        ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, n8 > 0 ? 0.6f : 0.5f);
        ahb2.d(n2, n3, n4, this);
        if (n7 == 1) {
            ahb2.d(n2 - 1, n3, n4, this);
        } else if (n7 == 2) {
            ahb2.d(n2 + 1, n3, n4, this);
        } else if (n7 == 3) {
            ahb2.d(n2, n3, n4 - 1, this);
        } else if (n7 == 4) {
            ahb2.d(n2, n3, n4 + 1, this);
        } else if (n7 == 5 || n7 == 6) {
            ahb2.d(n2, n3 - 1, n4, this);
        } else if (n7 == 0 || n7 == 7) {
            ahb2.d(n2, n3 + 1, n4, this);
        }
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        if ((n5 & 8) > 0) {
            ahb2.d(n2, n3, n4, this);
            int n6 = n5 & 7;
            if (n6 == 1) {
                ahb2.d(n2 - 1, n3, n4, this);
            } else if (n6 == 2) {
                ahb2.d(n2 + 1, n3, n4, this);
            } else if (n6 == 3) {
                ahb2.d(n2, n3, n4 - 1, this);
            } else if (n6 == 4) {
                ahb2.d(n2, n3, n4 + 1, this);
            } else if (n6 == 5 || n6 == 6) {
                ahb2.d(n2, n3 - 1, n4, this);
            } else if (n6 == 0 || n6 == 7) {
                ahb2.d(n2, n3 + 1, n4, this);
            }
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        return (ahl2.e(n2, n3, n4) & 8) > 0 ? 15 : 0;
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        int n6 = ahl2.e(n2, n3, n4);
        if ((n6 & 8) == 0) {
            return 0;
        }
        int n7 = n6 & 7;
        if (n7 == 0 && n5 == 0) {
            return 15;
        }
        if (n7 == 7 && n5 == 0) {
            return 15;
        }
        if (n7 == 6 && n5 == 1) {
            return 15;
        }
        if (n7 == 5 && n5 == 1) {
            return 15;
        }
        if (n7 == 4 && n5 == 2) {
            return 15;
        }
        if (n7 == 3 && n5 == 3) {
            return 15;
        }
        if (n7 == 2 && n5 == 4) {
            return 15;
        }
        if (n7 == 1 && n5 == 5) {
            return 15;
        }
        return 0;
    }

    @Override
    public boolean f() {
        return true;
    }
}

