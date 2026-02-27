/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aoc
extends aji {
    protected aoc() {
        super(awt.q);
        this.a(true);
        this.a(abt.c);
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
        return 2;
    }

    private boolean m(ahb ahb2, int n2, int n3, int n4) {
        if (ahb.a(ahb2, n2, n3, n4)) {
            return true;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        return aji2 == ajn.aJ || aji2 == ajn.bk || aji2 == ajn.w || aji2 == ajn.bK;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.c(n2 - 1, n3, n4, true)) {
            return true;
        }
        if (ahb2.c(n2 + 1, n3, n4, true)) {
            return true;
        }
        if (ahb2.c(n2, n3, n4 - 1, true)) {
            return true;
        }
        if (ahb2.c(n2, n3, n4 + 1, true)) {
            return true;
        }
        return this.m(ahb2, n2, n3 - 1, n4);
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = n6;
        if (n5 == 1 && this.m(ahb2, n2, n3 - 1, n4)) {
            n7 = 5;
        }
        if (n5 == 2 && ahb2.c(n2, n3, n4 + 1, true)) {
            n7 = 4;
        }
        if (n5 == 3 && ahb2.c(n2, n3, n4 - 1, true)) {
            n7 = 3;
        }
        if (n5 == 4 && ahb2.c(n2 + 1, n3, n4, true)) {
            n7 = 2;
        }
        if (n5 == 5 && ahb2.c(n2 - 1, n3, n4, true)) {
            n7 = 1;
        }
        return n7;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        super.a(ahb2, n2, n3, n4, random);
        if (ahb2.e(n2, n3, n4) == 0) {
            this.b(ahb2, n2, n3, n4);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.e(n2, n3, n4) == 0) {
            if (ahb2.c(n2 - 1, n3, n4, true)) {
                ahb2.a(n2, n3, n4, 1, 2);
            } else if (ahb2.c(n2 + 1, n3, n4, true)) {
                ahb2.a(n2, n3, n4, 2, 2);
            } else if (ahb2.c(n2, n3, n4 - 1, true)) {
                ahb2.a(n2, n3, n4, 3, 2);
            } else if (ahb2.c(n2, n3, n4 + 1, true)) {
                ahb2.a(n2, n3, n4, 4, 2);
            } else if (this.m(ahb2, n2, n3 - 1, n4)) {
                ahb2.a(n2, n3, n4, 5, 2);
            }
        }
        this.e(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.b(ahb2, n2, n3, n4, aji2);
    }

    protected boolean b(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (this.e(ahb2, n2, n3, n4)) {
            int n5 = ahb2.e(n2, n3, n4);
            boolean bl2 = false;
            if (!ahb2.c(n2 - 1, n3, n4, true) && n5 == 1) {
                bl2 = true;
            }
            if (!ahb2.c(n2 + 1, n3, n4, true) && n5 == 2) {
                bl2 = true;
            }
            if (!ahb2.c(n2, n3, n4 - 1, true) && n5 == 3) {
                bl2 = true;
            }
            if (!ahb2.c(n2, n3, n4 + 1, true) && n5 == 4) {
                bl2 = true;
            }
            if (!this.m(ahb2, n2, n3 - 1, n4) && n5 == 5) {
                bl2 = true;
            }
            if (bl2) {
                this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
                ahb2.f(n2, n3, n4);
                return true;
            }
        } else {
            return true;
        }
        return false;
    }

    protected boolean e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.c(ahb2, n2, n3, n4)) {
            if (ahb2.a(n2, n3, n4) == this) {
                this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
                ahb2.f(n2, n3, n4);
            }
            return false;
        }
        return true;
    }

    @Override
    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        int n5 = ahb2.e(n2, n3, n4) & 7;
        float f2 = 0.15f;
        if (n5 == 1) {
            this.a(0.0f, 0.2f, 0.5f - f2, f2 * 2.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 2) {
            this.a(1.0f - f2 * 2.0f, 0.2f, 0.5f - f2, 1.0f, 0.8f, 0.5f + f2);
        } else if (n5 == 3) {
            this.a(0.5f - f2, 0.2f, 0.0f, 0.5f + f2, 0.8f, f2 * 2.0f);
        } else if (n5 == 4) {
            this.a(0.5f - f2, 0.2f, 1.0f - f2 * 2.0f, 0.5f + f2, 0.8f, 1.0f);
        } else {
            f2 = 0.1f;
            this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.6f, 0.5f + f2);
        }
        return super.a(ahb2, n2, n3, n4, azw2, azw3);
    }
}

