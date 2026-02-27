/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class asl
extends arn {
    private aji a;

    public asl(aji aji2) {
        this.a = aji2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        if (!ahb2.c(n2, n3, n4) || ahb2.a(n2, n3 - 1, n4) != this.a) {
            return false;
        }
        int n9 = random.nextInt(32) + 6;
        int n10 = random.nextInt(4) + 1;
        for (n8 = n2 - n10; n8 <= n2 + n10; ++n8) {
            for (n7 = n4 - n10; n7 <= n4 + n10; ++n7) {
                n6 = n8 - n2;
                n5 = n7 - n4;
                if (n6 * n6 + n5 * n5 > n10 * n10 + 1 || ahb2.a(n8, n3 - 1, n7) == this.a) continue;
                return false;
            }
        }
        for (n8 = n3; n8 < n3 + n9 && n8 < 256; ++n8) {
            for (n7 = n2 - n10; n7 <= n2 + n10; ++n7) {
                for (n6 = n4 - n10; n6 <= n4 + n10; ++n6) {
                    n5 = n7 - n2;
                    int n11 = n6 - n4;
                    if (n5 * n5 + n11 * n11 > n10 * n10 + 1) continue;
                    ahb2.d(n7, n8, n6, ajn.Z, 0, 2);
                }
            }
        }
        wz wz2 = new wz(ahb2);
        wz2.b((float)n2 + 0.5f, n3 + n9, (float)n4 + 0.5f, random.nextFloat() * 360.0f, 0.0f);
        ahb2.d(wz2);
        ahb2.d(n2, n3 + n9, n4, ajn.h, 0, 2);
        return true;
    }
}

