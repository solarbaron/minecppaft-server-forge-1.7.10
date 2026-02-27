/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ajt
extends aji {
    protected ajt() {
        super(awt.A);
        this.a(true);
        this.a(abt.c);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
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
                    this.a(ahb2, n2, n3 + 1, n4, this);
                } else {
                    ahb2.a(n2, n3, n4, n6 + 1, 4);
                }
            }
        }
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        float f2 = 0.0625f;
        return azt.a((float)n2 + f2, n3, (float)n4 + f2, (float)(n2 + 1) - f2, (float)(n3 + 1) - f2, (float)(n4 + 1) - f2);
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public int b() {
        return 13;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (!super.c(ahb2, n2, n3, n4)) {
            return false;
        }
        return this.j(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!this.j(ahb2, n2, n3, n4)) {
            ahb2.a(n2, n3, n4, true);
        }
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2 - 1, n3, n4).o().a()) {
            return false;
        }
        if (ahb2.a(n2 + 1, n3, n4).o().a()) {
            return false;
        }
        if (ahb2.a(n2, n3, n4 - 1).o().a()) {
            return false;
        }
        if (ahb2.a(n2, n3, n4 + 1).o().a()) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3 - 1, n4);
        return aji2 == ajn.aF || aji2 == ajn.m;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        sa2.a(ro.g, 1.0f);
    }
}

