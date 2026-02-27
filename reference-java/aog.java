/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aog
extends aji {
    public aog() {
        super(awt.q);
        this.a(abt.d);
        this.a(true);
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
        return 29;
    }

    @Override
    public int a(ahb ahb2) {
        return 10;
    }

    @Override
    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
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
        return ahb2.a(n2, n3, n4 + 1).r();
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = 0;
        if (n5 == 2 && ahb2.c(n2, n3, n4 + 1, true)) {
            n7 = 2;
        }
        if (n5 == 3 && ahb2.c(n2, n3, n4 - 1, true)) {
            n7 = 0;
        }
        if (n5 == 4 && ahb2.c(n2 + 1, n3, n4, true)) {
            n7 = 1;
        }
        if (n5 == 5 && ahb2.c(n2 - 1, n3, n4, true)) {
            n7 = 3;
        }
        return n7;
    }

    @Override
    public void e(ahb ahb2, int n2, int n3, int n4, int n5) {
        this.a(ahb2, n2, n3, n4, false, n5, false, -1, 0);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (aji2 == this) {
            return;
        }
        if (this.e(ahb2, n2, n3, n4)) {
            int n5 = ahb2.e(n2, n3, n4);
            int n6 = n5 & 3;
            boolean bl2 = false;
            if (!ahb2.a(n2 - 1, n3, n4).r() && n6 == 3) {
                bl2 = true;
            }
            if (!ahb2.a(n2 + 1, n3, n4).r() && n6 == 1) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3, n4 - 1).r() && n6 == 0) {
                bl2 = true;
            }
            if (!ahb2.a(n2, n3, n4 + 1).r() && n6 == 2) {
                bl2 = true;
            }
            if (bl2) {
                this.b(ahb2, n2, n3, n4, n5, 0);
                ahb2.f(n2, n3, n4);
            }
        }
    }

    public void a(ahb ahb2, int n2, int n3, int n4, boolean bl2, int n5, boolean bl3, int n6, int n7) {
        int n8;
        int n9;
        int n10;
        int n11;
        int n12 = n5 & 3;
        boolean bl4 = (n5 & 4) == 4;
        boolean bl5 = (n5 & 8) == 8;
        boolean bl6 = !bl2;
        boolean bl7 = false;
        boolean bl8 = !ahb.a(ahb2, n2, n3 - 1, n4);
        int n13 = p.a[n12];
        int n14 = p.b[n12];
        int n15 = 0;
        int[] nArray = new int[42];
        for (n11 = 1; n11 < 42; ++n11) {
            n10 = n2 + n13 * n11;
            n9 = n4 + n14 * n11;
            aji aji2 = ahb2.a(n10, n3, n9);
            if (aji2 == ajn.bC) {
                n8 = ahb2.e(n10, n3, n9);
                if ((n8 & 3) != p.f[n12]) break;
                n15 = n11;
                break;
            }
            if (aji2 == ajn.bD || n11 == n6) {
                n8 = n11 == n6 ? n7 : ahb2.e(n10, n3, n9);
                boolean bl9 = (n8 & 8) != 8;
                boolean bl10 = (n8 & 1) == 1;
                boolean bl11 = (n8 & 2) == 2;
                bl6 &= bl11 == bl8;
                bl7 |= bl9 && bl10;
                nArray[n11] = n8;
                if (n11 != n6) continue;
                ahb2.a(n2, n3, n4, this, this.a(ahb2));
                bl6 &= bl9;
                continue;
            }
            nArray[n11] = -1;
            bl6 = false;
        }
        n11 = (bl6 ? 4 : 0) | ((bl7 &= (bl6 &= n15 > 1)) ? 8 : 0);
        n5 = n12 | n11;
        if (n15 > 0) {
            n10 = n2 + n13 * n15;
            n9 = n4 + n14 * n15;
            int n16 = p.f[n12];
            ahb2.a(n10, n3, n9, n16 | n11, 3);
            this.a(ahb2, n10, n3, n9, n16);
            this.a(ahb2, n10, n3, n9, bl6, bl7, bl4, bl5);
        }
        this.a(ahb2, n2, n3, n4, bl6, bl7, bl4, bl5);
        if (!bl2) {
            ahb2.a(n2, n3, n4, n5, 3);
            if (bl3) {
                this.a(ahb2, n2, n3, n4, n12);
            }
        }
        if (bl4 != bl6) {
            for (n10 = 1; n10 < n15; ++n10) {
                n9 = n2 + n13 * n10;
                int n17 = n4 + n14 * n10;
                n8 = nArray[n10];
                if (n8 < 0) continue;
                n8 = bl6 ? (n8 |= 4) : (n8 &= 0xFFFFFFFB);
                ahb2.a(n9, n3, n17, n8, 3);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        this.a(ahb2, n2, n3, n4, false, ahb2.e(n2, n3, n4), true, -1, 0);
    }

    private void a(ahb ahb2, int n2, int n3, int n4, boolean bl2, boolean bl3, boolean bl4, boolean bl5) {
        if (bl3 && !bl5) {
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.1, (double)n4 + 0.5, "random.click", 0.4f, 0.6f);
        } else if (!bl3 && bl5) {
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.1, (double)n4 + 0.5, "random.click", 0.4f, 0.5f);
        } else if (bl2 && !bl4) {
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.1, (double)n4 + 0.5, "random.click", 0.4f, 0.7f);
        } else if (!bl2 && bl4) {
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.1, (double)n4 + 0.5, "random.bowhit", 0.4f, 1.2f / (ahb2.s.nextFloat() * 0.2f + 0.9f));
        }
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        ahb2.d(n2, n3, n4, this);
        if (n5 == 3) {
            ahb2.d(n2 - 1, n3, n4, this);
        } else if (n5 == 1) {
            ahb2.d(n2 + 1, n3, n4, this);
        } else if (n5 == 0) {
            ahb2.d(n2, n3, n4 - 1, this);
        } else if (n5 == 2) {
            ahb2.d(n2, n3, n4 + 1, this);
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
        int n5 = ahl2.e(n2, n3, n4) & 3;
        float f2 = 0.1875f;
        if (n5 == 3) {
            this.a(0.0f, 0.2f, 0.5f - f2, f2 * 2.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 1) {
            this.a(1.0f - f2 * 2.0f, 0.2f, 0.5f - f2, 1.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 0) {
            this.a(0.5f - f2, 0.2f, 0.0f, 0.5f + f2, 0.8f, f2 * 2.0f);
        } else if (n5 == 2) {
            this.a(0.5f - f2, 0.2f, 1.0f - f2 * 2.0f, 0.5f + f2, 0.8f, 1.0f);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        boolean bl2;
        boolean bl3 = (n5 & 4) == 4;
        boolean bl4 = bl2 = (n5 & 8) == 8;
        if (bl3 || bl2) {
            this.a(ahb2, n2, n3, n4, true, n5, false, -1, 0);
        }
        if (bl2) {
            ahb2.d(n2, n3, n4, this);
            int n6 = n5 & 3;
            if (n6 == 3) {
                ahb2.d(n2 - 1, n3, n4, this);
            } else if (n6 == 1) {
                ahb2.d(n2 + 1, n3, n4, this);
            } else if (n6 == 0) {
                ahb2.d(n2, n3, n4 - 1, this);
            } else if (n6 == 2) {
                ahb2.d(n2, n3, n4 + 1, this);
            }
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        return (ahl2.e(n2, n3, n4) & 8) == 8 ? 15 : 0;
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        int n6 = ahl2.e(n2, n3, n4);
        if ((n6 & 8) != 8) {
            return 0;
        }
        int n7 = n6 & 3;
        if (n7 == 2 && n5 == 2) {
            return 15;
        }
        if (n7 == 0 && n5 == 3) {
            return 15;
        }
        if (n7 == 1 && n5 == 4) {
            return 15;
        }
        if (n7 == 3 && n5 == 5) {
            return 15;
        }
        return 0;
    }

    @Override
    public boolean f() {
        return true;
    }
}

