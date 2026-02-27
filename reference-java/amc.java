/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amc
extends ajr
implements ajo {
    protected amc() {
        float f2 = 0.2f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f2 * 2.0f, 0.5f + f2);
        this.a(true);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (random.nextInt(25) == 0) {
            int n5;
            int n6;
            int n7;
            int n8 = 4;
            int n9 = 5;
            for (n7 = n2 - n8; n7 <= n2 + n8; ++n7) {
                for (n6 = n4 - n8; n6 <= n4 + n8; ++n6) {
                    for (n5 = n3 - 1; n5 <= n3 + 1; ++n5) {
                        if (ahb2.a(n7, n5, n6) != this || --n9 > 0) continue;
                        return;
                    }
                }
            }
            n7 = n2 + random.nextInt(3) - 1;
            n6 = n3 + random.nextInt(2) - random.nextInt(2);
            n5 = n4 + random.nextInt(3) - 1;
            for (int i2 = 0; i2 < 4; ++i2) {
                if (ahb2.c(n7, n6, n5) && this.j(ahb2, n7, n6, n5)) {
                    n2 = n7;
                    n3 = n6;
                    n4 = n5;
                }
                n7 = n2 + random.nextInt(3) - 1;
                n6 = n3 + random.nextInt(2) - random.nextInt(2);
                n5 = n4 + random.nextInt(3) - 1;
            }
            if (ahb2.c(n7, n6, n5) && this.j(ahb2, n7, n6, n5)) {
                ahb2.d(n7, n6, n5, this, 0, 2);
            }
        }
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return super.c(ahb2, n2, n3, n4) && this.j(ahb2, n2, n3, n4);
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2.j();
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        if (n3 < 0 || n3 >= 256) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3 - 1, n4);
        return aji2 == ajn.bh || aji2 == ajn.d && ahb2.e(n2, n3 - 1, n4) == 2 || ahb2.j(n2, n3, n4) < 13 && this.a(aji2);
    }

    public boolean c(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4);
        ahb2.f(n2, n3, n4);
        aru aru2 = null;
        if (this == ajn.P) {
            aru2 = new aru(0);
        } else if (this == ajn.Q) {
            aru2 = new aru(1);
        }
        if (aru2 == null || !((arn)aru2).a(ahb2, random, n2, n3, n4)) {
            ahb2.d(n2, n3, n4, this, n5, 3);
            return false;
        }
        return true;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        return true;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return (double)random.nextFloat() < 0.4;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        this.c(ahb2, n2, n3, n4, random);
    }
}

