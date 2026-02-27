/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amp
extends alk {
    public static final int[][] a = new int[][]{new int[0], {3, 1}, {2, 0}};

    public amp() {
        super("portal", awt.E, false);
        this.a(true);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        super.a(ahb2, n2, n3, n4, random);
        if (ahb2.t.d() && ahb2.O().b("doMobSpawning") && random.nextInt(2000) < ahb2.r.a()) {
            sa sa2;
            int n5;
            for (n5 = n3; !ahb.a(ahb2, n2, n5, n4) && n5 > 0; --n5) {
            }
            if (n5 > 0 && !ahb2.a(n2, n5 + 1, n4).r() && (sa2 = aee.a(ahb2, 57, (double)n2 + 0.5, (double)n5 + 1.1, (double)n4 + 0.5)) != null) {
                sa2.am = sa2.ai();
            }
        }
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = amp.b(ahl2.e(n2, n3, n4));
        if (n5 == 0) {
            n5 = ahl2.a(n2 - 1, n3, n4) == this || ahl2.a(n2 + 1, n3, n4) == this ? 1 : 2;
            if (ahl2 instanceof ahb && !((ahb)ahl2).E) {
                ((ahb)ahl2).a(n2, n3, n4, n5, 2);
            }
        }
        float f2 = 0.125f;
        float f3 = 0.125f;
        if (n5 == 1) {
            f2 = 0.5f;
        }
        if (n5 == 2) {
            f3 = 0.5f;
        }
        this.a(0.5f - f2, 0.0f, 0.5f - f3, 0.5f + f2, 1.0f, 0.5f + f3);
    }

    @Override
    public boolean d() {
        return false;
    }

    public boolean e(ahb ahb2, int n2, int n3, int n4) {
        amq amq2 = new amq(ahb2, n2, n3, n4, 1);
        amq amq3 = new amq(ahb2, n2, n3, n4, 2);
        if (amq2.b() && amq.a(amq2) == 0) {
            amq2.c();
            return true;
        }
        if (amq3.b() && amq.a(amq3) == 0) {
            amq3.c();
            return true;
        }
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = amp.b(ahb2.e(n2, n3, n4));
        amq amq2 = new amq(ahb2, n2, n3, n4, 1);
        amq amq3 = new amq(ahb2, n2, n3, n4, 2);
        if (!(n5 != 1 || amq2.b() && amq.a(amq2) >= amq.b(amq2) * amq.c(amq2))) {
            ahb2.b(n2, n3, n4, ajn.a);
        } else if (!(n5 != 2 || amq3.b() && amq.a(amq3) >= amq.b(amq3) * amq.c(amq3))) {
            ahb2.b(n2, n3, n4, ajn.a);
        } else if (n5 == 0 && !amq2.b() && !amq3.b()) {
            ahb2.b(n2, n3, n4, ajn.a);
        }
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        if (sa2.m == null && sa2.l == null) {
            sa2.ah();
        }
    }

    public static int b(int n2) {
        return n2 & 3;
    }
}

