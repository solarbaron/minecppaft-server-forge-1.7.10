/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class are
extends arc {
    private boolean a;

    public are(boolean bl2, boolean bl3) {
        super(bl2);
        this.a = bl3;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8 = random.nextInt(3) + 5;
        if (this.a) {
            n8 += random.nextInt(7);
        }
        boolean bl2 = true;
        if (n3 < 1 || n3 + n8 + 1 > 256) {
            return false;
        }
        for (int i2 = n3; i2 <= n3 + 1 + n8; ++i2) {
            n7 = 1;
            if (i2 == n3) {
                n7 = 0;
            }
            if (i2 >= n3 + 1 + n8 - 2) {
                n7 = 2;
            }
            for (n6 = n2 - n7; n6 <= n2 + n7 && bl2; ++n6) {
                for (n5 = n4 - n7; n5 <= n4 + n7 && bl2; ++n5) {
                    if (i2 >= 0 && i2 < 256) {
                        aji aji2 = ahb2.a(n6, i2, n5);
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
        if (aji3 != ajn.c && aji3 != ajn.d && aji3 != ajn.ak || n3 >= 256 - n8 - 1) {
            return false;
        }
        this.a(ahb2, n2, n3 - 1, n4, ajn.d);
        for (n7 = n3 - 3 + n8; n7 <= n3 + n8; ++n7) {
            n6 = n7 - (n3 + n8);
            n5 = 1 - n6 / 2;
            for (int i3 = n2 - n5; i3 <= n2 + n5; ++i3) {
                int n9 = i3 - n2;
                for (int i4 = n4 - n5; i4 <= n4 + n5; ++i4) {
                    aji aji4;
                    int n10 = i4 - n4;
                    if (Math.abs(n9) == n5 && Math.abs(n10) == n5 && (random.nextInt(2) == 0 || n6 == 0) || (aji4 = ahb2.a(i3, n7, i4)).o() != awt.a && aji4.o() != awt.j) continue;
                    this.a(ahb2, i3, n7, i4, ajn.t, 2);
                }
            }
        }
        for (n7 = 0; n7 < n8; ++n7) {
            aji aji5 = ahb2.a(n2, n3 + n7, n4);
            if (aji5.o() != awt.a && aji5.o() != awt.j) continue;
            this.a(ahb2, n2, n3 + n7, n4, ajn.r, 2);
        }
        return true;
    }
}

