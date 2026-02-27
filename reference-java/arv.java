/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arv
extends arn {
    private aji a = ajn.cj;
    private int b;

    public arv(int n2) {
        this.b = n2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        while (ahb2.c(n2, n3, n4) && n3 > 2) {
            --n3;
        }
        if (ahb2.a(n2, n3, n4) != ajn.aE) {
            return false;
        }
        int n5 = random.nextInt(this.b - 2) + 2;
        int n6 = 1;
        for (int i2 = n2 - n5; i2 <= n2 + n5; ++i2) {
            for (int i3 = n4 - n5; i3 <= n4 + n5; ++i3) {
                int n7 = i2 - n2;
                int n8 = i3 - n4;
                if (n7 * n7 + n8 * n8 > n5 * n5) continue;
                for (int i4 = n3 - n6; i4 <= n3 + n6; ++i4) {
                    aji aji2 = ahb2.a(i2, i4, i3);
                    if (aji2 != ajn.d && aji2 != ajn.aE && aji2 != ajn.aD) continue;
                    ahb2.d(i2, i4, i3, this.a, 0, 2);
                }
            }
        }
        return true;
    }
}

