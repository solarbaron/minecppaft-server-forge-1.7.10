/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akf
extends ajr
implements ajo {
    protected akf() {
        this.a(true);
        float f2 = 0.5f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.25f, 0.5f + f2);
        this.a((abt)null);
        this.c(0.0f);
        this.a(h);
        this.H();
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2 == ajn.ak;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        float f2;
        int n5;
        super.a(ahb2, n2, n3, n4, random);
        if (ahb2.k(n2, n3 + 1, n4) >= 9 && (n5 = ahb2.e(n2, n3, n4)) < 7 && random.nextInt((int)(25.0f / (f2 = this.n(ahb2, n2, n3, n4))) + 1) == 0) {
            ahb2.a(n2, n3, n4, ++n5, 2);
        }
    }

    public void m(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4) + qh.a(ahb2.s, 2, 5);
        if (n5 > 7) {
            n5 = 7;
        }
        ahb2.a(n2, n3, n4, n5, 2);
    }

    private float n(ahb ahb2, int n2, int n3, int n4) {
        float f2 = 1.0f;
        aji aji2 = ahb2.a(n2, n3, n4 - 1);
        aji aji3 = ahb2.a(n2, n3, n4 + 1);
        aji aji4 = ahb2.a(n2 - 1, n3, n4);
        aji aji5 = ahb2.a(n2 + 1, n3, n4);
        aji aji6 = ahb2.a(n2 - 1, n3, n4 - 1);
        aji aji7 = ahb2.a(n2 + 1, n3, n4 - 1);
        aji aji8 = ahb2.a(n2 + 1, n3, n4 + 1);
        aji aji9 = ahb2.a(n2 - 1, n3, n4 + 1);
        boolean bl2 = aji4 == this || aji5 == this;
        boolean bl3 = aji2 == this || aji3 == this;
        boolean bl4 = aji6 == this || aji7 == this || aji8 == this || aji9 == this;
        for (int i2 = n2 - 1; i2 <= n2 + 1; ++i2) {
            for (int i3 = n4 - 1; i3 <= n4 + 1; ++i3) {
                float f3 = 0.0f;
                if (ahb2.a(i2, n3 - 1, i3) == ajn.ak) {
                    f3 = 1.0f;
                    if (ahb2.e(i2, n3 - 1, i3) > 0) {
                        f3 = 3.0f;
                    }
                }
                if (i2 != n2 || i3 != n4) {
                    f3 /= 4.0f;
                }
                f2 += f3;
            }
        }
        if (bl4 || bl2 && bl3) {
            f2 /= 2.0f;
        }
        return f2;
    }

    @Override
    public int b() {
        return 6;
    }

    protected adb i() {
        return ade.N;
    }

    protected adb P() {
        return ade.O;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, 0);
        if (ahb2.E) {
            return;
        }
        if (n5 >= 7) {
            int n7 = 3 + n6;
            for (int i2 = 0; i2 < n7; ++i2) {
                if (ahb2.s.nextInt(15) > n5) continue;
                this.a(ahb2, n2, n3, n4, new add(this.i(), 1, 0));
            }
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (n2 == 7) {
            return this.P();
        }
        return this.i();
    }

    @Override
    public int a(Random random) {
        return 1;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        return ahb2.e(n2, n3, n4) != 7;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        this.m(ahb2, n2, n3, n4);
    }
}

