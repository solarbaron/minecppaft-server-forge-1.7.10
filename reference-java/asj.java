/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class asj
extends arn {
    private aji a;
    private int b;

    public asj(aji aji2, int n2) {
        this.a = aji2;
        this.b = n2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4).o() != awt.h) {
            return false;
        }
        int n5 = random.nextInt(this.b - 2) + 2;
        int n6 = 2;
        for (int i2 = n2 - n5; i2 <= n2 + n5; ++i2) {
            for (int i3 = n4 - n5; i3 <= n4 + n5; ++i3) {
                int n7 = i2 - n2;
                int n8 = i3 - n4;
                if (n7 * n7 + n8 * n8 > n5 * n5) continue;
                for (int i4 = n3 - n6; i4 <= n3 + n6; ++i4) {
                    aji aji2 = ahb2.a(i2, i4, i3);
                    if (aji2 != ajn.d && aji2 != ajn.c) continue;
                    ahb2.d(i2, i4, i3, this.a, 0, 2);
                }
            }
        }
        return true;
    }
}

