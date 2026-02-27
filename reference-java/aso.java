/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aso
extends arc {
    public aso() {
        super(false);
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8;
        int n9 = random.nextInt(4) + 5;
        while (ahb2.a(n2, n3 - 1, n4).o() == awt.h) {
            --n3;
        }
        boolean bl2 = true;
        if (n3 < 1 || n3 + n9 + 1 > 256) {
            return false;
        }
        for (int i2 = n3; i2 <= n3 + 1 + n9; ++i2) {
            n8 = 1;
            if (i2 == n3) {
                n8 = 0;
            }
            if (i2 >= n3 + 1 + n9 - 2) {
                n8 = 3;
            }
            for (n7 = n2 - n8; n7 <= n2 + n8 && bl2; ++n7) {
                for (n6 = n4 - n8; n6 <= n4 + n8 && bl2; ++n6) {
                    if (i2 >= 0 && i2 < 256) {
                        aji aji2 = ahb2.a(n7, i2, n6);
                        if (aji2.o() == awt.a || aji2.o() == awt.j) continue;
                        if (aji2 == ajn.j || aji2 == ajn.i) {
                            if (i2 <= n3) continue;
                            bl2 = false;
                            continue;
                        }
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
        if (aji3 != ajn.c && aji3 != ajn.d || n3 >= 256 - n9 - 1) {
            return false;
        }
        this.a(ahb2, n2, n3 - 1, n4, ajn.d);
        for (n8 = n3 - 3 + n9; n8 <= n3 + n9; ++n8) {
            n7 = n8 - (n3 + n9);
            n6 = 2 - n7 / 2;
            for (int i3 = n2 - n6; i3 <= n2 + n6; ++i3) {
                n5 = i3 - n2;
                for (int i4 = n4 - n6; i4 <= n4 + n6; ++i4) {
                    int n10 = i4 - n4;
                    if (Math.abs(n5) == n6 && Math.abs(n10) == n6 && (random.nextInt(2) == 0 || n7 == 0) || ahb2.a(i3, n8, i4).j()) continue;
                    this.a(ahb2, i3, n8, i4, ajn.t);
                }
            }
        }
        for (n8 = 0; n8 < n9; ++n8) {
            aji aji4 = ahb2.a(n2, n3 + n8, n4);
            if (aji4.o() != awt.a && aji4.o() != awt.j && aji4 != ajn.i && aji4 != ajn.j) continue;
            this.a(ahb2, n2, n3 + n8, n4, ajn.r);
        }
        for (n8 = n3 - 3 + n9; n8 <= n3 + n9; ++n8) {
            int n11 = n8 - (n3 + n9);
            n6 = 2 - n11 / 2;
            for (int i5 = n2 - n6; i5 <= n2 + n6; ++i5) {
                for (n5 = n4 - n6; n5 <= n4 + n6; ++n5) {
                    if (ahb2.a(i5, n8, n5).o() != awt.j) continue;
                    if (random.nextInt(4) == 0 && ahb2.a(i5 - 1, n8, n5).o() == awt.a) {
                        this.a(ahb2, i5 - 1, n8, n5, 8);
                    }
                    if (random.nextInt(4) == 0 && ahb2.a(i5 + 1, n8, n5).o() == awt.a) {
                        this.a(ahb2, i5 + 1, n8, n5, 2);
                    }
                    if (random.nextInt(4) == 0 && ahb2.a(i5, n8, n5 - 1).o() == awt.a) {
                        this.a(ahb2, i5, n8, n5 - 1, 1);
                    }
                    if (random.nextInt(4) != 0 || ahb2.a(i5, n8, n5 + 1).o() != awt.a) continue;
                    this.a(ahb2, i5, n8, n5 + 1, 4);
                }
            }
        }
        return true;
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        this.a(ahb2, n2, n3, n4, ajn.bd, n5);
        for (int i2 = 4; ahb2.a(n2, --n3, n4).o() == awt.a && i2 > 0; --i2) {
            this.a(ahb2, n2, n3, n4, ajn.bd, n5);
        }
    }
}

