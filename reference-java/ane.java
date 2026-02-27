/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ane
extends aji {
    protected ane() {
        super(awt.k);
        float f2 = 0.375f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 1.0f, 0.5f + f2);
        this.a(true);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.a(n2, n3 - 1, n4) != ajn.aH && !this.e(ahb2, n2, n3, n4)) {
            return;
        }
        if (ahb2.c(n2, n3 + 1, n4)) {
            int n5 = 1;
            while (ahb2.a(n2, n3 - n5, n4) == this) {
                ++n5;
            }
            if (n5 < 3) {
                int n6 = ahb2.e(n2, n3, n4);
                if (n6 == 15) {
                    ahb2.b(n2, n3 + 1, n4, this);
                    ahb2.a(n2, n3, n4, 0, 4);
                } else {
                    ahb2.a(n2, n3, n4, n6 + 1, 4);
                }
            }
        }
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3 - 1, n4);
        if (aji2 == this) {
            return true;
        }
        if (aji2 != ajn.c && aji2 != ajn.d && aji2 != ajn.m) {
            return false;
        }
        if (ahb2.a(n2 - 1, n3 - 1, n4).o() == awt.h) {
            return true;
        }
        if (ahb2.a(n2 + 1, n3 - 1, n4).o() == awt.h) {
            return true;
        }
        if (ahb2.a(n2, n3 - 1, n4 - 1).o() == awt.h) {
            return true;
        }
        return ahb2.a(n2, n3 - 1, n4 + 1).o() == awt.h;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.e(ahb2, n2, n3, n4);
    }

    protected final boolean e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            return false;
        }
        return true;
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return this.c(ahb2, n2, n3, n4);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.aE;
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
        return 1;
    }
}

