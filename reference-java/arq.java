/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arq
extends arn {
    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        for (int i2 = 0; i2 < 64; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + random.nextInt(8) - random.nextInt(8);
            if (!ahb2.c(n7, n6 = n3 + random.nextInt(4) - random.nextInt(4), n5 = n4 + random.nextInt(8) - random.nextInt(8)) || ahb2.a(n7, n6 - 1, n5) != ajn.aL) continue;
            ahb2.d(n7, n6, n5, ajn.ab, 0, 2);
        }
        return true;
    }
}

