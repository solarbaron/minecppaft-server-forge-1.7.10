/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ark
extends arn {
    private aji a;

    public ark(aji aji2) {
        this.a = aji2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        aji aji2;
        while (((aji2 = ahb2.a(n2, n3, n4)).o() == awt.a || aji2.o() == awt.j) && n3 > 0) {
            --n3;
        }
        for (int i2 = 0; i2 < 4; ++i2) {
            int n5;
            int n6;
            int n7 = n2 + random.nextInt(8) - random.nextInt(8);
            if (!ahb2.c(n7, n6 = n3 + random.nextInt(4) - random.nextInt(4), n5 = n4 + random.nextInt(8) - random.nextInt(8)) || !this.a.j(ahb2, n7, n6, n5)) continue;
            ahb2.d(n7, n6, n5, this.a, 0, 2);
        }
        return true;
    }
}

