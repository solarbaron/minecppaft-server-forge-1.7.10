/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amz
extends aji {
    private boolean a;

    public amz(boolean bl2) {
        super(awt.e);
        if (bl2) {
            this.a(true);
        }
        this.a = bl2;
    }

    @Override
    public int a(ahb ahb2) {
        return 30;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        this.e(ahb2, n2, n3, n4);
        super.a(ahb2, n2, n3, n4, yz2);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        this.e(ahb2, n2, n3, n4);
        super.b(ahb2, n2, n3, n4, sa2);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        this.e(ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4, yz2, n5, f2, f3, f4);
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        this.m(ahb2, n2, n3, n4);
        if (this == ajn.ax) {
            ahb2.b(n2, n3, n4, ajn.ay);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (this == ajn.ay) {
            ahb2.b(n2, n3, n4, ajn.ax);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ax;
    }

    @Override
    public int a(int n2, Random random) {
        return this.a(random) + random.nextInt(n2 + 1);
    }

    @Override
    public int a(Random random) {
        return 4 + random.nextInt(2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, n6);
        if (this.a(n5, ahb2.s, n6) != adb.a(this)) {
            int n7 = 1 + ahb2.s.nextInt(5);
            this.c(ahb2, n2, n3, n4, n7);
        }
    }

    private void m(ahb ahb2, int n2, int n3, int n4) {
        Random random = ahb2.s;
        double d2 = 0.0625;
        for (int i2 = 0; i2 < 6; ++i2) {
            double d3 = (float)n2 + random.nextFloat();
            double d4 = (float)n3 + random.nextFloat();
            double d5 = (float)n4 + random.nextFloat();
            if (i2 == 0 && !ahb2.a(n2, n3 + 1, n4).c()) {
                d4 = (double)(n3 + 1) + d2;
            }
            if (i2 == 1 && !ahb2.a(n2, n3 - 1, n4).c()) {
                d4 = (double)(n3 + 0) - d2;
            }
            if (i2 == 2 && !ahb2.a(n2, n3, n4 + 1).c()) {
                d5 = (double)(n4 + 1) + d2;
            }
            if (i2 == 3 && !ahb2.a(n2, n3, n4 - 1).c()) {
                d5 = (double)(n4 + 0) - d2;
            }
            if (i2 == 4 && !ahb2.a(n2 + 1, n3, n4).c()) {
                d3 = (double)(n2 + 1) + d2;
            }
            if (i2 == 5 && !ahb2.a(n2 - 1, n3, n4).c()) {
                d3 = (double)(n2 + 0) - d2;
            }
            if (!(d3 < (double)n2 || d3 > (double)(n2 + 1) || d4 < 0.0 || d4 > (double)(n3 + 1) || d5 < (double)n4) && !(d5 > (double)(n4 + 1))) continue;
            ahb2.a("reddust", d3, d4, d5, 0.0, 0.0, 0.0);
        }
    }

    @Override
    protected add j(int n2) {
        return new add(ajn.ax);
    }
}

