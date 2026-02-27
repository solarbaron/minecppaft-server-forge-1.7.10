/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anz
extends ajr
implements ajo {
    private static final String[] a = new String[]{"deadbush", "tallgrass", "fern"};

    protected anz() {
        super(awt.l);
        float f2 = 0.4f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.8f, 0.5f + f2);
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return this.a(ahb2.a(n2, n3 - 1, n4));
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (random.nextInt(8) == 0) {
            return ade.N;
        }
        return null;
    }

    @Override
    public int a(int n2, Random random) {
        return 1 + random.nextInt(n2 * 2 + 1);
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E && yz2.bF() != null && yz2.bF().b() == ade.aZ) {
            yz2.a(pp.C[aji.b(this)], 1);
            this.a(ahb2, n2, n3, n4, new add(ajn.H, 1, n5));
        } else {
            super.a(ahb2, yz2, n2, n3, n4, n5);
        }
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.e(n2, n3, n4);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        int n5 = ahb2.e(n2, n3, n4);
        return n5 != 0;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = 2;
        if (n5 == 2) {
            n6 = 3;
        }
        if (ajn.cm.c(ahb2, n2, n3, n4)) {
            ajn.cm.c(ahb2, n2, n3, n4, n6, 2);
        }
    }
}

