/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akp
extends aji {
    public akp() {
        super(awt.D);
        this.a(0.0625f, 0.0f, 0.0625f, 0.9375f, 1.0f, 0.9375f);
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
        this.e(ahb2, n2, n3, n4);
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        if (akx.e(ahb2, n2, n3 - 1, n4) && n3 >= 0) {
            int n5 = 32;
            if (akx.M || !ahb2.b(n2 - n5, n3 - n5, n4 - n5, n2 + n5, n3 + n5, n4 + n5)) {
                ahb2.f(n2, n3, n4);
                while (akx.e(ahb2, n2, n3 - 1, n4) && n3 > 0) {
                    --n3;
                }
                if (n3 > 0) {
                    ahb2.d(n2, n3, n4, this, 0, 2);
                }
            } else {
                xj xj2 = new xj(ahb2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, this);
                ahb2.d(xj2);
            }
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        this.m(ahb2, n2, n3, n4);
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        this.m(ahb2, n2, n3, n4);
    }

    private void m(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4) != this) {
            return;
        }
        for (int i2 = 0; i2 < 1000; ++i2) {
            int n5 = n2 + ahb2.s.nextInt(16) - ahb2.s.nextInt(16);
            int n6 = n3 + ahb2.s.nextInt(8) - ahb2.s.nextInt(8);
            int n7 = n4 + ahb2.s.nextInt(16) - ahb2.s.nextInt(16);
            if (ahb2.a((int)n5, (int)n6, (int)n7).J != awt.a) continue;
            if (!ahb2.E) {
                ahb2.d(n5, n6, n7, this, ahb2.e(n2, n3, n4), 2);
                ahb2.f(n2, n3, n4);
            } else {
                int n8 = 128;
                for (int i3 = 0; i3 < n8; ++i3) {
                    double d2 = ahb2.s.nextDouble();
                    float f2 = (ahb2.s.nextFloat() - 0.5f) * 0.2f;
                    float f3 = (ahb2.s.nextFloat() - 0.5f) * 0.2f;
                    float f4 = (ahb2.s.nextFloat() - 0.5f) * 0.2f;
                    double d3 = (double)n5 + (double)(n2 - n5) * d2 + (ahb2.s.nextDouble() - 0.5) * 1.0 + 0.5;
                    double d4 = (double)n6 + (double)(n3 - n6) * d2 + ahb2.s.nextDouble() * 1.0 - 0.5;
                    double d5 = (double)n7 + (double)(n4 - n7) * d2 + (ahb2.s.nextDouble() - 0.5) * 1.0 + 0.5;
                    ahb2.a("portal", d3, d4, d5, (double)f2, (double)f3, f4);
                }
            }
            return;
        }
    }

    @Override
    public int a(ahb ahb2) {
        return 5;
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
        return 27;
    }
}

