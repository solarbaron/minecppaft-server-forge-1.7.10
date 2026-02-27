/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arz
extends asb {
    public arz(boolean bl2, int n2, int n3, int n4, int n5) {
        super(bl2, n2, n3, n4, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5 = this.a(random);
        if (!this.a(ahb2, random, n2, n3, n4, n5)) {
            return false;
        }
        this.c(ahb2, n2, n4, n3 + n5, 2, random);
        for (int i2 = n3 + n5 - 2 - random.nextInt(4); i2 > n3 + n5 / 2; i2 -= 2 + random.nextInt(4)) {
            int n6;
            float f2 = random.nextFloat() * (float)Math.PI * 2.0f;
            int n7 = n2 + (int)(0.5f + qh.b(f2) * 4.0f);
            int n8 = n4 + (int)(0.5f + qh.a(f2) * 4.0f);
            for (n6 = 0; n6 < 5; ++n6) {
                n7 = n2 + (int)(1.5f + qh.b(f2) * (float)n6);
                n8 = n4 + (int)(1.5f + qh.a(f2) * (float)n6);
                this.a(ahb2, n7, i2 - 3 + n6 / 2, n8, ajn.r, this.b);
            }
            n6 = 1 + random.nextInt(2);
            int n9 = i2;
            for (int i3 = n9 - n6; i3 <= n9; ++i3) {
                int n10 = i3 - n9;
                this.b(ahb2, n7, i3, n8, 1 - n10, random);
            }
        }
        for (int i4 = 0; i4 < n5; ++i4) {
            aji aji2 = ahb2.a(n2, n3 + i4, n4);
            if (aji2.o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2, n3 + i4, n4, ajn.r, this.b);
                if (i4 > 0) {
                    if (random.nextInt(3) > 0 && ahb2.c(n2 - 1, n3 + i4, n4)) {
                        this.a(ahb2, n2 - 1, n3 + i4, n4, ajn.bd, 8);
                    }
                    if (random.nextInt(3) > 0 && ahb2.c(n2, n3 + i4, n4 - 1)) {
                        this.a(ahb2, n2, n3 + i4, n4 - 1, ajn.bd, 1);
                    }
                }
            }
            if (i4 >= n5 - 1) continue;
            aji2 = ahb2.a(n2 + 1, n3 + i4, n4);
            if (aji2.o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2 + 1, n3 + i4, n4, ajn.r, this.b);
                if (i4 > 0) {
                    if (random.nextInt(3) > 0 && ahb2.c(n2 + 2, n3 + i4, n4)) {
                        this.a(ahb2, n2 + 2, n3 + i4, n4, ajn.bd, 2);
                    }
                    if (random.nextInt(3) > 0 && ahb2.c(n2 + 1, n3 + i4, n4 - 1)) {
                        this.a(ahb2, n2 + 1, n3 + i4, n4 - 1, ajn.bd, 1);
                    }
                }
            }
            if ((aji2 = ahb2.a(n2 + 1, n3 + i4, n4 + 1)).o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2 + 1, n3 + i4, n4 + 1, ajn.r, this.b);
                if (i4 > 0) {
                    if (random.nextInt(3) > 0 && ahb2.c(n2 + 2, n3 + i4, n4 + 1)) {
                        this.a(ahb2, n2 + 2, n3 + i4, n4 + 1, ajn.bd, 2);
                    }
                    if (random.nextInt(3) > 0 && ahb2.c(n2 + 1, n3 + i4, n4 + 2)) {
                        this.a(ahb2, n2 + 1, n3 + i4, n4 + 2, ajn.bd, 4);
                    }
                }
            }
            if ((aji2 = ahb2.a(n2, n3 + i4, n4 + 1)).o() != awt.a && aji2.o() != awt.j) continue;
            this.a(ahb2, n2, n3 + i4, n4 + 1, ajn.r, this.b);
            if (i4 <= 0) continue;
            if (random.nextInt(3) > 0 && ahb2.c(n2 - 1, n3 + i4, n4 + 1)) {
                this.a(ahb2, n2 - 1, n3 + i4, n4 + 1, ajn.bd, 8);
            }
            if (random.nextInt(3) <= 0 || !ahb2.c(n2, n3 + i4, n4 + 2)) continue;
            this.a(ahb2, n2, n3 + i4, n4 + 2, ajn.bd, 4);
        }
        return true;
    }

    private void c(ahb ahb2, int n2, int n3, int n4, int n5, Random random) {
        int n6 = 2;
        for (int i2 = n4 - n6; i2 <= n4; ++i2) {
            int n7 = i2 - n4;
            this.a(ahb2, n2, i2, n3, n5 + 1 - n7, random);
        }
    }
}

