/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class asi
extends arc {
    public asi(boolean bl2) {
        super(bl2);
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        int n9;
        int n10 = random.nextInt(3) + random.nextInt(2) + 6;
        boolean bl2 = true;
        if (n3 < 1 || n3 + n10 + 1 > 256) {
            return false;
        }
        for (int i2 = n3; i2 <= n3 + 1 + n10; ++i2) {
            n9 = 1;
            if (i2 == n3) {
                n9 = 0;
            }
            if (i2 >= n3 + 1 + n10 - 2) {
                n9 = 2;
            }
            for (n8 = n2 - n9; n8 <= n2 + n9 && bl2; ++n8) {
                for (n7 = n4 - n9; n7 <= n4 + n9 && bl2; ++n7) {
                    if (i2 >= 0 && i2 < 256) {
                        aji aji2 = ahb2.a(n8, i2, n7);
                        if (this.a(aji2)) continue;
                        bl2 = false;
                        continue;
                    }
                    bl2 = false;
                }
            }
        }
        if (!bl2) {
            return false;
        }
        aji aji3 = ahb2.a(n2, n3 - 1, n4);
        if (aji3 != ajn.c && aji3 != ajn.d || n3 >= 256 - n10 - 1) {
            return false;
        }
        this.a(ahb2, n2, n3 - 1, n4, ajn.d);
        this.a(ahb2, n2 + 1, n3 - 1, n4, ajn.d);
        this.a(ahb2, n2 + 1, n3 - 1, n4 + 1, ajn.d);
        this.a(ahb2, n2, n3 - 1, n4 + 1, ajn.d);
        n9 = random.nextInt(4);
        n8 = n10 - random.nextInt(4);
        n7 = 2 - random.nextInt(3);
        int n11 = n2;
        int n12 = n4;
        int n13 = 0;
        for (n6 = 0; n6 < n10; ++n6) {
            aji aji4;
            n5 = n3 + n6;
            if (n6 >= n8 && n7 > 0) {
                n11 += p.a[n9];
                n12 += p.b[n9];
                --n7;
            }
            if ((aji4 = ahb2.a(n11, n5, n12)).o() != awt.a && aji4.o() != awt.j) continue;
            this.a(ahb2, n11, n5, n12, ajn.s, 1);
            this.a(ahb2, n11 + 1, n5, n12, ajn.s, 1);
            this.a(ahb2, n11, n5, n12 + 1, ajn.s, 1);
            this.a(ahb2, n11 + 1, n5, n12 + 1, ajn.s, 1);
            n13 = n5;
        }
        for (n6 = -2; n6 <= 0; ++n6) {
            for (n5 = -2; n5 <= 0; ++n5) {
                int n14 = -1;
                this.a(ahb2, n11 + n6, n13 + n14, n12 + n5);
                this.a(ahb2, 1 + n11 - n6, n13 + n14, n12 + n5);
                this.a(ahb2, n11 + n6, n13 + n14, 1 + n12 - n5);
                this.a(ahb2, 1 + n11 - n6, n13 + n14, 1 + n12 - n5);
                if (n6 <= -2 && n5 <= -1 || n6 == -1 && n5 == -2) continue;
                n14 = 1;
                this.a(ahb2, n11 + n6, n13 + n14, n12 + n5);
                this.a(ahb2, 1 + n11 - n6, n13 + n14, n12 + n5);
                this.a(ahb2, n11 + n6, n13 + n14, 1 + n12 - n5);
                this.a(ahb2, 1 + n11 - n6, n13 + n14, 1 + n12 - n5);
            }
        }
        if (random.nextBoolean()) {
            this.a(ahb2, n11, n13 + 2, n12);
            this.a(ahb2, n11 + 1, n13 + 2, n12);
            this.a(ahb2, n11 + 1, n13 + 2, n12 + 1);
            this.a(ahb2, n11, n13 + 2, n12 + 1);
        }
        for (n6 = -3; n6 <= 4; ++n6) {
            for (n5 = -3; n5 <= 4; ++n5) {
                if (n6 == -3 && n5 == -3 || n6 == -3 && n5 == 4 || n6 == 4 && n5 == -3 || n6 == 4 && n5 == 4 || Math.abs(n6) >= 3 && Math.abs(n5) >= 3) continue;
                this.a(ahb2, n11 + n6, n13, n12 + n5);
            }
        }
        for (n6 = -1; n6 <= 2; ++n6) {
            for (n5 = -1; n5 <= 2; ++n5) {
                int n15;
                int n16;
                if (n6 >= 0 && n6 <= 1 && n5 >= 0 && n5 <= 1 || random.nextInt(3) > 0) continue;
                int n17 = random.nextInt(3) + 2;
                for (n16 = 0; n16 < n17; ++n16) {
                    this.a(ahb2, n2 + n6, n13 - n16 - 1, n4 + n5, ajn.s, 1);
                }
                for (n16 = -1; n16 <= 1; ++n16) {
                    for (n15 = -1; n15 <= 1; ++n15) {
                        this.a(ahb2, n11 + n6 + n16, n13 - 0, n12 + n5 + n15);
                    }
                }
                for (n16 = -2; n16 <= 2; ++n16) {
                    for (n15 = -2; n15 <= 2; ++n15) {
                        if (Math.abs(n16) == 2 && Math.abs(n15) == 2) continue;
                        this.a(ahb2, n11 + n6 + n16, n13 - 1, n12 + n5 + n15);
                    }
                }
            }
        }
        return true;
    }

    private void a(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2.o() == awt.a) {
            this.a(ahb2, n2, n3, n4, ajn.u, 1);
        }
    }
}

