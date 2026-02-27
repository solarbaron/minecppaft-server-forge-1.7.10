/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arg
extends arn {
    private final qx[] a;
    private final int b;

    public arg(qx[] qxArray, int n2) {
        this.a = qxArray;
        this.b = n2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        aji aji2;
        while (((aji2 = ahb2.a(n2, n3, n4)).o() == awt.a || aji2.o() == awt.j) && n3 > 1) {
            --n3;
        }
        if (n3 < 1) {
            return false;
        }
        ++n3;
        for (int i2 = 0; i2 < 4; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + random.nextInt(4) - random.nextInt(4);
            if (!ahb2.c(n7, n6 = n3 + random.nextInt(3) - random.nextInt(3), n5 = n4 + random.nextInt(4) - random.nextInt(4)) || !ahb.a(ahb2, n7, n6 - 1, n5)) continue;
            ahb2.d(n7, n6, n5, ajn.ae, 0, 2);
            aow aow2 = (aow)ahb2.o(n7, n6, n5);
            if (aow2 != null && aow2 != null) {
                qx.a(random, this.a, aow2, this.b);
            }
            if (ahb2.c(n7 - 1, n6, n5) && ahb.a(ahb2, n7 - 1, n6 - 1, n5)) {
                ahb2.d(n7 - 1, n6, n5, ajn.aa, 0, 2);
            }
            if (ahb2.c(n7 + 1, n6, n5) && ahb.a(ahb2, n7 - 1, n6 - 1, n5)) {
                ahb2.d(n7 + 1, n6, n5, ajn.aa, 0, 2);
            }
            if (ahb2.c(n7, n6, n5 - 1) && ahb.a(ahb2, n7 - 1, n6 - 1, n5)) {
                ahb2.d(n7, n6, n5 - 1, ajn.aa, 0, 2);
            }
            if (ahb2.c(n7, n6, n5 + 1) && ahb.a(ahb2, n7 - 1, n6 - 1, n5)) {
                ahb2.d(n7, n6, n5 + 1, ajn.aa, 0, 2);
            }
            return true;
        }
        return false;
    }
}

