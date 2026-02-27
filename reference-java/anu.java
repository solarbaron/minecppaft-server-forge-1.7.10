/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anu
extends ajr
implements ajo {
    private final aji a;

    protected anu(aji aji2) {
        this.a = aji2;
        this.a(true);
        float f2 = 0.125f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.25f, 0.5f + f2);
        this.a((abt)null);
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2 == ajn.ak;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        float f2;
        super.a(ahb2, n2, n3, n4, random);
        if (ahb2.k(n2, n3 + 1, n4) >= 9 && random.nextInt((int)(25.0f / (f2 = this.n(ahb2, n2, n3, n4))) + 1) == 0) {
            int n5 = ahb2.e(n2, n3, n4);
            if (n5 < 7) {
                ahb2.a(n2, n3, n4, ++n5, 2);
            } else {
                if (ahb2.a(n2 - 1, n3, n4) == this.a) {
                    return;
                }
                if (ahb2.a(n2 + 1, n3, n4) == this.a) {
                    return;
                }
                if (ahb2.a(n2, n3, n4 - 1) == this.a) {
                    return;
                }
                if (ahb2.a(n2, n3, n4 + 1) == this.a) {
                    return;
                }
                int n6 = random.nextInt(4);
                int n7 = n2;
                int n8 = n4;
                if (n6 == 0) {
                    --n7;
                }
                if (n6 == 1) {
                    ++n7;
                }
                if (n6 == 2) {
                    --n8;
                }
                if (n6 == 3) {
                    ++n8;
                }
                aji aji2 = ahb2.a(n7, n3 - 1, n8);
                if (ahb2.a((int)n7, (int)n3, (int)n8).J == awt.a && (aji2 == ajn.ak || aji2 == ajn.d || aji2 == ajn.c)) {
                    ahb2.b(n7, n3, n8, this.a);
                }
            }
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
                aji aji10 = ahb2.a(i2, n3 - 1, i3);
                float f3 = 0.0f;
                if (aji10 == ajn.ak) {
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
    public void g() {
        float f2 = 0.125f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.25f, 0.5f + f2);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.F = (float)(ahl2.e(n2, n3, n4) * 2 + 2) / 16.0f;
        float f2 = 0.125f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, (float)this.F, 0.5f + f2);
    }

    @Override
    public int b() {
        return 19;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, n6);
        if (ahb2.E) {
            return;
        }
        adb adb2 = null;
        if (this.a == ajn.aK) {
            adb2 = ade.bb;
        }
        if (this.a == ajn.ba) {
            adb2 = ade.bc;
        }
        for (int i2 = 0; i2 < 3; ++i2) {
            if (ahb2.s.nextInt(15) > n5) continue;
            this.a(ahb2, n2, n3, n4, new add(adb2));
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
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

