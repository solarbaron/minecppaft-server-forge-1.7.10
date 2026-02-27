/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public abstract class ajs
extends aji {
    private final boolean a;

    protected ajs(boolean bl2) {
        super(awt.q);
        this.a(true);
        this.a(abt.d);
        this.a = bl2;
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public int a(ahb ahb2) {
        return this.a ? 30 : 20;
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
        int n7 = ahb2.e(n2, n3, n4);
        int n8 = n7 & 8;
        n7 &= 7;
        n7 = n5 == 2 && ahb2.a(n2, n3, n4 + 1).r() ? 4 : (n5 == 3 && ahb2.a(n2, n3, n4 - 1).r() ? 3 : (n5 == 4 && ahb2.a(n2 + 1, n3, n4).r() ? 2 : (n5 == 5 && ahb2.a(n2 - 1, n3, n4).r() ? 1 : this.e(ahb2, n2, n3, n4))));
        return n7 + n8;
    }

    private int e(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2 - 1, n3, n4).r()) {
            return 1;
        }
        if (ahb2.a(n2 + 1, n3, n4).r()) {
            return 2;
        }
        if (ahb2.a(n2, n3, n4 - 1).r()) {
            return 3;
        }
        if (ahb2.a(n2, n3, n4 + 1).r()) {
            return 4;
        }
        return 1;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (this.m(ahb2, n2, n3, n4)) {
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
            if (bl2) {
                this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
                ahb2.f(n2, n3, n4);
            }
        }
    }

    private boolean m(ahb ahb2, int n2, int n3, int n4) {
        if (!this.c(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            return false;
        }
        return true;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        this.b(n5);
    }

    private void b(int n2) {
        int n3 = n2 & 7;
        boolean bl2 = (n2 & 8) > 0;
        float f2 = 0.375f;
        float f3 = 0.625f;
        float f4 = 0.1875f;
        float f5 = 0.125f;
        if (bl2) {
            f5 = 0.0625f;
        }
        if (n3 == 1) {
            this.a(0.0f, f2, 0.5f - f4, f5, f3, 0.5f + f4);
        } else if (n3 == 2) {
            this.a(1.0f - f5, f2, 0.5f - f4, 1.0f, f3, 0.5f + f4);
        } else if (n3 == 3) {
            this.a(0.5f - f4, f2, 0.0f, 0.5f + f4, f3, f5);
        } else if (n3 == 4) {
            this.a(0.5f - f4, f2, 1.0f - f5, 0.5f + f4, f3, 1.0f);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = n6 & 7;
        int n8 = 8 - (n6 & 8);
        if (n8 == 0) {
            return true;
        }
        ahb2.a(n2, n3, n4, n7 + n8, 3);
        ahb2.c(n2, n3, n4, n2, n3, n4);
        ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, 0.6f);
        this.a(ahb2, n2, n3, n4, n7);
        ahb2.a(n2, n3, n4, this, this.a(ahb2));
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        if ((n5 & 8) > 0) {
            int n6 = n5 & 7;
            this.a(ahb2, n2, n3, n4, n6);
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

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) == 0) {
            return;
        }
        if (this.a) {
            this.n(ahb2, n2, n3, n4);
        } else {
            ahb2.a(n2, n3, n4, n5 & 7, 3);
            int n6 = n5 & 7;
            this.a(ahb2, n2, n3, n4, n6);
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, 0.5f);
            ahb2.c(n2, n3, n4, n2, n3, n4);
        }
    }

    @Override
    public void g() {
        float f2 = 0.1875f;
        float f3 = 0.125f;
        float f4 = 0.125f;
        this.a(0.5f - f2, 0.5f - f3, 0.5f - f4, 0.5f + f2, 0.5f + f3, 0.5f + f4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        if (ahb2.E) {
            return;
        }
        if (!this.a) {
            return;
        }
        if ((ahb2.e(n2, n3, n4) & 8) != 0) {
            return;
        }
        this.n(ahb2, n2, n3, n4);
    }

    private void n(ahb ahb2, int n2, int n3, int n4) {
        boolean bl2;
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = n5 & 7;
        boolean bl3 = (n5 & 8) != 0;
        this.b(n5);
        List list = ahb2.a(zc.class, azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (double)n3 + this.F, (double)n4 + this.G));
        boolean bl4 = bl2 = !list.isEmpty();
        if (bl2 && !bl3) {
            ahb2.a(n2, n3, n4, n6 | 8, 3);
            this.a(ahb2, n2, n3, n4, n6);
            ahb2.c(n2, n3, n4, n2, n3, n4);
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, 0.6f);
        }
        if (!bl2 && bl3) {
            ahb2.a(n2, n3, n4, n6, 3);
            this.a(ahb2, n2, n3, n4, n6);
            ahb2.c(n2, n3, n4, n2, n3, n4);
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, 0.5f);
        }
        if (bl2) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
        }
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        ahb2.d(n2, n3, n4, this);
        if (n5 == 1) {
            ahb2.d(n2 - 1, n3, n4, this);
        } else if (n5 == 2) {
            ahb2.d(n2 + 1, n3, n4, this);
        } else if (n5 == 3) {
            ahb2.d(n2, n3, n4 - 1, this);
        } else if (n5 == 4) {
            ahb2.d(n2, n3, n4 + 1, this);
        } else {
            ahb2.d(n2, n3 - 1, n4, this);
        }
    }
}

