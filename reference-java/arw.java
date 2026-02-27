/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arw
extends arn {
    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        while (ahb2.c(n2, n3, n4) && n3 > 2) {
            --n3;
        }
        if (ahb2.a(n2, n3, n4) != ajn.aE) {
            return false;
        }
        n3 += random.nextInt(4);
        int n8 = random.nextInt(4) + 7;
        int n9 = n8 / 4 + random.nextInt(2);
        if (n9 > 1 && random.nextInt(60) == 0) {
            n3 += 10 + random.nextInt(30);
        }
        for (n7 = 0; n7 < n8; ++n7) {
            float f2 = (1.0f - (float)n7 / (float)n8) * (float)n9;
            n6 = qh.f(f2);
            for (n5 = -n6; n5 <= n6; ++n5) {
                float f3 = (float)qh.a(n5) - 0.25f;
                for (int i2 = -n6; i2 <= n6; ++i2) {
                    float f4 = (float)qh.a(i2) - 0.25f;
                    if ((n5 != 0 || i2 != 0) && f3 * f3 + f4 * f4 > f2 * f2 || (n5 == -n6 || n5 == n6 || i2 == -n6 || i2 == n6) && random.nextFloat() > 0.75f) continue;
                    aji aji2 = ahb2.a(n2 + n5, n3 + n7, n4 + i2);
                    if (aji2.o() == awt.a || aji2 == ajn.d || aji2 == ajn.aE || aji2 == ajn.aD) {
                        this.a(ahb2, n2 + n5, n3 + n7, n4 + i2, ajn.cj);
                    }
                    if (n7 == 0 || n6 <= 1 || (aji2 = ahb2.a(n2 + n5, n3 - n7, n4 + i2)).o() != awt.a && aji2 != ajn.d && aji2 != ajn.aE && aji2 != ajn.aD) continue;
                    this.a(ahb2, n2 + n5, n3 - n7, n4 + i2, ajn.cj);
                }
            }
        }
        n7 = n9 - 1;
        if (n7 < 0) {
            n7 = 0;
        } else if (n7 > 1) {
            n7 = 1;
        }
        for (int i3 = -n7; i3 <= n7; ++i3) {
            for (n6 = -n7; n6 <= n7; ++n6) {
                aji aji3;
                n5 = n3 - 1;
                int n10 = 50;
                if (Math.abs(i3) == 1 && Math.abs(n6) == 1) {
                    n10 = random.nextInt(5);
                }
                while (n5 > 50 && ((aji3 = ahb2.a(n2 + i3, n5, n4 + n6)).o() == awt.a || aji3 == ajn.d || aji3 == ajn.aE || aji3 == ajn.aD || aji3 == ajn.cj)) {
                    this.a(ahb2, n2 + i3, n5, n4 + n6, ajn.cj);
                    --n5;
                    if (--n10 > 0) continue;
                    n5 -= random.nextInt(5) + 1;
                    n10 = random.nextInt(5);
                }
            }
        }
        return true;
    }
}

