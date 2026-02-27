/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ash
extends arn {
    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        for (int i2 = 0; i2 < 20; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + random.nextInt(4) - random.nextInt(4);
            if (!ahb2.c(n7, n6 = n3, n5 = n4 + random.nextInt(4) - random.nextInt(4)) || ahb2.a(n7 - 1, n6 - 1, n5).o() != awt.h && ahb2.a(n7 + 1, n6 - 1, n5).o() != awt.h && ahb2.a(n7, n6 - 1, n5 - 1).o() != awt.h && ahb2.a(n7, n6 - 1, n5 + 1).o() != awt.h) continue;
            int n8 = 2 + random.nextInt(random.nextInt(3) + 1);
            for (int i3 = 0; i3 < n8; ++i3) {
                if (!ajn.aH.j(ahb2, n7, n6 + i3, n5)) continue;
                ahb2.d(n7, n6 + i3, n5, ajn.aH, 0, 2);
            }
        }
        return true;
    }
}

