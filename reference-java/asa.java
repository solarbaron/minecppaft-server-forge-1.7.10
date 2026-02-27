/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class asa
extends asb {
    private boolean e;

    public asa(boolean bl2, boolean bl3) {
        super(bl2, 13, 15, 1, 1);
        this.e = bl3;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5 = this.a(random);
        if (!this.a(ahb2, random, n2, n3, n4, n5)) {
            return false;
        }
        this.c(ahb2, n2, n4, n3 + n5, 0, random);
        for (int i2 = 0; i2 < n5; ++i2) {
            aji aji2 = ahb2.a(n2, n3 + i2, n4);
            if (aji2.o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2, n3 + i2, n4, ajn.r, this.b);
            }
            if (i2 >= n5 - 1) continue;
            aji2 = ahb2.a(n2 + 1, n3 + i2, n4);
            if (aji2.o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2 + 1, n3 + i2, n4, ajn.r, this.b);
            }
            if ((aji2 = ahb2.a(n2 + 1, n3 + i2, n4 + 1)).o() == awt.a || aji2.o() == awt.j) {
                this.a(ahb2, n2 + 1, n3 + i2, n4 + 1, ajn.r, this.b);
            }
            if ((aji2 = ahb2.a(n2, n3 + i2, n4 + 1)).o() != awt.a && aji2.o() != awt.j) continue;
            this.a(ahb2, n2, n3 + i2, n4 + 1, ajn.r, this.b);
        }
        return true;
    }

    private void c(ahb ahb2, int n2, int n3, int n4, int n5, Random random) {
        int n6 = random.nextInt(5);
        n6 = this.e ? (n6 += this.a) : (n6 += 3);
        int n7 = 0;
        for (int i2 = n4 - n6; i2 <= n4; ++i2) {
            int n8 = n4 - i2;
            int n9 = n5 + qh.d((float)n8 / (float)n6 * 3.5f);
            this.a(ahb2, n2, i2, n3, n9 + (n8 > 0 && n9 == n7 && (i2 & 1) == 0 ? 1 : 0), random);
            n7 = n9;
        }
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        this.c(ahb2, random, n2 - 1, n3, n4 - 1);
        this.c(ahb2, random, n2 + 2, n3, n4 - 1);
        this.c(ahb2, random, n2 - 1, n3, n4 + 2);
        this.c(ahb2, random, n2 + 2, n3, n4 + 2);
        for (int i2 = 0; i2 < 5; ++i2) {
            int n5 = random.nextInt(64);
            int n6 = n5 % 8;
            int n7 = n5 / 8;
            if (n6 != 0 && n6 != 7 && n7 != 0 && n7 != 7) continue;
            this.c(ahb2, random, n2 - 3 + n6, n3, n4 - 3 + n7);
        }
    }

    private void c(ahb ahb2, Random random, int n2, int n3, int n4) {
        for (int i2 = -2; i2 <= 2; ++i2) {
            for (int i3 = -2; i3 <= 2; ++i3) {
                if (Math.abs(i2) == 2 && Math.abs(i3) == 2) continue;
                this.a(ahb2, n2 + i2, n3, n4 + i3);
            }
        }
    }

    private void a(ahb ahb2, int n2, int n3, int n4) {
        for (int i2 = n3 + 2; i2 >= n3 - 3; --i2) {
            aji aji2 = ahb2.a(n2, i2, n4);
            if (aji2 == ajn.c || aji2 == ajn.d) {
                this.a(ahb2, n2, i2, n4, ajn.d, 2);
                break;
            }
            if (aji2.o() != awt.a && i2 < n3) break;
        }
    }
}

