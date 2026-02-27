/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amd
extends aji {
    protected amd() {
        super(awt.b);
        this.a(true);
        this.a(abt.b);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        if (ahb2.k(n2, n3 + 1, n4) < 4 && ahb2.a(n2, n3 + 1, n4).k() > 2) {
            ahb2.b(n2, n3, n4, ajn.d);
        } else if (ahb2.k(n2, n3 + 1, n4) >= 9) {
            for (int i2 = 0; i2 < 4; ++i2) {
                int n5 = n2 + random.nextInt(3) - 1;
                int n6 = n3 + random.nextInt(5) - 3;
                int n7 = n4 + random.nextInt(3) - 1;
                aji aji2 = ahb2.a(n5, n6 + 1, n7);
                if (ahb2.a(n5, n6, n7) != ajn.d || ahb2.e(n5, n6, n7) != 0 || ahb2.k(n5, n6 + 1, n7) < 4 || aji2.k() > 2) continue;
                ahb2.b(n5, n6, n7, this);
            }
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ajn.d.a(0, random, n3);
    }
}

