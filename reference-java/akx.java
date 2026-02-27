/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akx
extends aji {
    public static boolean M;

    public akx() {
        super(awt.p);
        this.a(abt.b);
    }

    public akx(awt awt2) {
        super(awt2);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        ahb2.a(n2, n3, n4, this, this.a(ahb2));
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        ahb2.a(n2, n3, n4, this, this.a(ahb2));
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (!ahb2.E) {
            this.m(ahb2, n2, n3, n4);
        }
    }

    private void m(ahb ahb2, int n2, int n3, int n4) {
        int n5 = n2;
        int n6 = n3;
        int n7 = n4;
        if (akx.e(ahb2, n5, n6 - 1, n7) && n6 >= 0) {
            int n8 = 32;
            if (M || !ahb2.b(n2 - n8, n3 - n8, n4 - n8, n2 + n8, n3 + n8, n4 + n8)) {
                ahb2.f(n2, n3, n4);
                while (akx.e(ahb2, n2, n3 - 1, n4) && n3 > 0) {
                    --n3;
                }
                if (n3 > 0) {
                    ahb2.b(n2, n3, n4, this);
                }
            } else if (!ahb2.E) {
                xj xj2 = new xj(ahb2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, this, ahb2.e(n2, n3, n4));
                this.a(xj2);
                ahb2.d(xj2);
            }
        }
    }

    protected void a(xj xj2) {
    }

    @Override
    public int a(ahb ahb2) {
        return 2;
    }

    public static boolean e(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2.J == awt.a) {
            return true;
        }
        if (aji2 == ajn.ab) {
            return true;
        }
        awt awt2 = aji2.J;
        if (awt2 == awt.h) {
            return true;
        }
        return awt2 == awt.i;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, int n5) {
    }
}

