/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arf
extends arn {
    private aji a;
    private int b;

    public arf(aji aji2, int n2) {
        super(false);
        this.a = aji2;
        this.b = n2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        aji aji2;
        while (n3 > 3 && (ahb2.c(n2, n3 - 1, n4) || (aji2 = ahb2.a(n2, n3 - 1, n4)) != ajn.c && aji2 != ajn.d && aji2 != ajn.b)) {
            --n3;
        }
        if (n3 <= 3) {
            return false;
        }
        int n5 = this.b;
        for (int i2 = 0; n5 >= 0 && i2 < 3; ++i2) {
            int n6 = n5 + random.nextInt(2);
            int n7 = n5 + random.nextInt(2);
            int n8 = n5 + random.nextInt(2);
            float f2 = (float)(n6 + n7 + n8) * 0.333f + 0.5f;
            for (int i3 = n2 - n6; i3 <= n2 + n6; ++i3) {
                for (int i4 = n4 - n8; i4 <= n4 + n8; ++i4) {
                    for (int i5 = n3 - n7; i5 <= n3 + n7; ++i5) {
                        float f3 = i3 - n2;
                        float f4 = i4 - n4;
                        float f5 = i5 - n3;
                        if (f3 * f3 + f4 * f4 + f5 * f5 > f2 * f2) continue;
                        ahb2.d(i3, i5, i4, this.a, 0, 4);
                    }
                }
            }
            n2 += -(n5 + 1) + random.nextInt(2 + n5 * 2);
            n4 += -(n5 + 1) + random.nextInt(2 + n5 * 2);
            n3 += 0 - random.nextInt(2);
        }
        return true;
    }
}

