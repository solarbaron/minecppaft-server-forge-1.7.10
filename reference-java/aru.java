/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aru
extends arn {
    private int a = -1;

    public aru(int n2) {
        super(true);
        this.a = n2;
    }

    public aru() {
        super(false);
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8 = random.nextInt(2);
        if (this.a >= 0) {
            n8 = this.a;
        }
        int n9 = random.nextInt(3) + 4;
        boolean bl2 = true;
        if (n3 < 1 || n3 + n9 + 1 >= 256) {
            return false;
        }
        for (int i2 = n3; i2 <= n3 + 1 + n9; ++i2) {
            n7 = 3;
            if (i2 <= n3 + 3) {
                n7 = 0;
            }
            for (n6 = n2 - n7; n6 <= n2 + n7 && bl2; ++n6) {
                for (n5 = n4 - n7; n5 <= n4 + n7 && bl2; ++n5) {
                    if (i2 >= 0 && i2 < 256) {
                        aji aji2 = ahb2.a(n6, i2, n5);
                        if (aji2.o() == awt.a || aji2.o() == awt.j) continue;
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
        if (aji3 != ajn.d && aji3 != ajn.c && aji3 != ajn.bh) {
            return false;
        }
        n7 = n3 + n9;
        if (n8 == 1) {
            n7 = n3 + n9 - 3;
        }
        for (n6 = n7; n6 <= n3 + n9; ++n6) {
            n5 = 1;
            if (n6 < n3 + n9) {
                ++n5;
            }
            if (n8 == 0) {
                n5 = 3;
            }
            for (int i3 = n2 - n5; i3 <= n2 + n5; ++i3) {
                for (int i4 = n4 - n5; i4 <= n4 + n5; ++i4) {
                    int n10 = 5;
                    if (i3 == n2 - n5) {
                        --n10;
                    }
                    if (i3 == n2 + n5) {
                        ++n10;
                    }
                    if (i4 == n4 - n5) {
                        n10 -= 3;
                    }
                    if (i4 == n4 + n5) {
                        n10 += 3;
                    }
                    if (n8 == 0 || n6 < n3 + n9) {
                        if ((i3 == n2 - n5 || i3 == n2 + n5) && (i4 == n4 - n5 || i4 == n4 + n5)) continue;
                        if (i3 == n2 - (n5 - 1) && i4 == n4 - n5) {
                            n10 = 1;
                        }
                        if (i3 == n2 - n5 && i4 == n4 - (n5 - 1)) {
                            n10 = 1;
                        }
                        if (i3 == n2 + (n5 - 1) && i4 == n4 - n5) {
                            n10 = 3;
                        }
                        if (i3 == n2 + n5 && i4 == n4 - (n5 - 1)) {
                            n10 = 3;
                        }
                        if (i3 == n2 - (n5 - 1) && i4 == n4 + n5) {
                            n10 = 7;
                        }
                        if (i3 == n2 - n5 && i4 == n4 + (n5 - 1)) {
                            n10 = 7;
                        }
                        if (i3 == n2 + (n5 - 1) && i4 == n4 + n5) {
                            n10 = 9;
                        }
                        if (i3 == n2 + n5 && i4 == n4 + (n5 - 1)) {
                            n10 = 9;
                        }
                    }
                    if (n10 == 5 && n6 < n3 + n9) {
                        n10 = 0;
                    }
                    if (n10 == 0 && n3 < n3 + n9 - 1 || ahb2.a(i3, n6, i4).j()) continue;
                    this.a(ahb2, i3, n6, i4, aji.e(aji.b(ajn.aW) + n8), n10);
                }
            }
        }
        for (n6 = 0; n6 < n9; ++n6) {
            aji aji4 = ahb2.a(n2, n3 + n6, n4);
            if (aji4.j()) continue;
            this.a(ahb2, n2, n3 + n6, n4, aji.e(aji.b(ajn.aW) + n8), 10);
        }
        return true;
    }
}

