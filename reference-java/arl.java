/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arl
extends arn {
    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        while (ahb2.c(n2, n3, n4) && n3 > 2) {
            --n3;
        }
        if (ahb2.a(n2, n3, n4) != ajn.m) {
            return false;
        }
        for (n6 = -2; n6 <= 2; ++n6) {
            for (n5 = -2; n5 <= 2; ++n5) {
                if (!ahb2.c(n2 + n6, n3 - 1, n4 + n5) || !ahb2.c(n2 + n6, n3 - 2, n4 + n5)) continue;
                return false;
            }
        }
        for (n6 = -1; n6 <= 0; ++n6) {
            for (n5 = -2; n5 <= 2; ++n5) {
                for (int i2 = -2; i2 <= 2; ++i2) {
                    ahb2.d(n2 + n5, n3 + n6, n4 + i2, ajn.A, 0, 2);
                }
            }
        }
        ahb2.d(n2, n3, n4, ajn.i, 0, 2);
        ahb2.d(n2 - 1, n3, n4, ajn.i, 0, 2);
        ahb2.d(n2 + 1, n3, n4, ajn.i, 0, 2);
        ahb2.d(n2, n3, n4 - 1, ajn.i, 0, 2);
        ahb2.d(n2, n3, n4 + 1, ajn.i, 0, 2);
        for (n6 = -2; n6 <= 2; ++n6) {
            for (n5 = -2; n5 <= 2; ++n5) {
                if (n6 != -2 && n6 != 2 && n5 != -2 && n5 != 2) continue;
                ahb2.d(n2 + n6, n3 + 1, n4 + n5, ajn.A, 0, 2);
            }
        }
        ahb2.d(n2 + 2, n3 + 1, n4, ajn.U, 1, 2);
        ahb2.d(n2 - 2, n3 + 1, n4, ajn.U, 1, 2);
        ahb2.d(n2, n3 + 1, n4 + 2, ajn.U, 1, 2);
        ahb2.d(n2, n3 + 1, n4 - 2, ajn.U, 1, 2);
        for (n6 = -1; n6 <= 1; ++n6) {
            for (n5 = -1; n5 <= 1; ++n5) {
                if (n6 == 0 && n5 == 0) {
                    ahb2.d(n2 + n6, n3 + 4, n4 + n5, ajn.A, 0, 2);
                    continue;
                }
                ahb2.d(n2 + n6, n3 + 4, n4 + n5, ajn.U, 1, 2);
            }
        }
        for (n6 = 1; n6 <= 3; ++n6) {
            ahb2.d(n2 - 1, n3 + n6, n4 - 1, ajn.A, 0, 2);
            ahb2.d(n2 - 1, n3 + n6, n4 + 1, ajn.A, 0, 2);
            ahb2.d(n2 + 1, n3 + n6, n4 - 1, ajn.A, 0, 2);
            ahb2.d(n2 + 1, n3 + n6, n4 + 1, ajn.A, 0, 2);
        }
        return true;
    }
}

