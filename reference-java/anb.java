/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anb
extends aji {
    private final boolean a;

    public anb(boolean bl2) {
        super(awt.t);
        this.a = bl2;
        if (bl2) {
            this.a(1.0f);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb2.E) {
            if (this.a && !ahb2.v(n2, n3, n4)) {
                ahb2.a(n2, n3, n4, this, 4);
            } else if (!this.a && ahb2.v(n2, n3, n4)) {
                ahb2.d(n2, n3, n4, ajn.bv, 0, 2);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb2.E) {
            if (this.a && !ahb2.v(n2, n3, n4)) {
                ahb2.a(n2, n3, n4, this, 4);
            } else if (!this.a && ahb2.v(n2, n3, n4)) {
                ahb2.d(n2, n3, n4, ajn.bv, 0, 2);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (!ahb2.E && this.a && !ahb2.v(n2, n3, n4)) {
            ahb2.d(n2, n3, n4, ajn.bu, 0, 2);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.bu);
    }

    @Override
    protected add j(int n2) {
        return new add(ajn.bu);
    }
}

