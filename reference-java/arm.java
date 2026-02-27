/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arm
extends arn {
    private int a;

    public void a(int n2) {
        this.a = n2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        boolean bl2 = false;
        for (int i2 = 0; i2 < 64; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + random.nextInt(8) - random.nextInt(8);
            if (!ahb2.c(n7, n6 = n3 + random.nextInt(4) - random.nextInt(4), n5 = n4 + random.nextInt(8) - random.nextInt(8)) || ahb2.t.g && n6 >= 254 || !ajn.cm.c(ahb2, n7, n6, n5)) continue;
            ajn.cm.c(ahb2, n7, n6, n5, this.a, 2);
            bl2 = true;
        }
        return bl2;
    }
}

