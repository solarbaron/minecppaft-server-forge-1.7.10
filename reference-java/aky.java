/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aky
extends aji {
    protected aky() {
        super(awt.c);
        this.a(true);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.9375f, 1.0f);
        this.g(255);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return azt.a(n2 + 0, n3 + 0, n4 + 0, n2 + 1, n3 + 1, n4 + 1);
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
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (this.m(ahb2, n2, n3, n4) || ahb2.y(n2, n3 + 1, n4)) {
            ahb2.a(n2, n3, n4, 7, 2);
        } else {
            int n5 = ahb2.e(n2, n3, n4);
            if (n5 > 0) {
                ahb2.a(n2, n3, n4, n5 - 1, 2);
            } else if (!this.e(ahb2, n2, n3, n4)) {
                ahb2.b(n2, n3, n4, ajn.d);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2, float f2) {
        if (!ahb2.E && ahb2.s.nextFloat() < f2 - 0.5f) {
            if (!(sa2 instanceof yz) && !ahb2.O().b("mobGriefing")) {
                return;
            }
            ahb2.b(n2, n3, n4, ajn.d);
        }
    }

    private boolean e(ahb ahb2, int n2, int n3, int n4) {
        int n5 = 0;
        for (int i2 = n2 - n5; i2 <= n2 + n5; ++i2) {
            for (int i3 = n4 - n5; i3 <= n4 + n5; ++i3) {
                aji aji2 = ahb2.a(i2, n3 + 1, i3);
                if (aji2 != ajn.aj && aji2 != ajn.bc && aji2 != ajn.bb && aji2 != ajn.bN && aji2 != ajn.bM) continue;
                return true;
            }
        }
        return false;
    }

    private boolean m(ahb ahb2, int n2, int n3, int n4) {
        for (int i2 = n2 - 4; i2 <= n2 + 4; ++i2) {
            for (int i3 = n3; i3 <= n3 + 1; ++i3) {
                for (int i4 = n4 - 4; i4 <= n4 + 4; ++i4) {
                    if (ahb2.a(i2, i3, i4).o() != awt.h) continue;
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        super.a(ahb2, n2, n3, n4, aji2);
        awt awt2 = ahb2.a(n2, n3 + 1, n4).o();
        if (awt2.a()) {
            ahb2.b(n2, n3, n4, ajn.d);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ajn.d.a(0, random, n3);
    }
}

