/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class alx
extends ang {
    public alx() {
        super(awt.d);
        this.a(abt.b);
        this.c(2.0f);
        this.a(f);
    }

    public static int c(int n2) {
        return n2 & 3;
    }

    @Override
    public int a(Random random) {
        return 1;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(this);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        int n6 = 4;
        int n7 = n6 + 1;
        if (ahb2.b(n2 - n7, n3 - n7, n4 - n7, n2 + n7, n3 + n7, n4 + n7)) {
            for (int i2 = -n6; i2 <= n6; ++i2) {
                for (int i3 = -n6; i3 <= n6; ++i3) {
                    for (int i4 = -n6; i4 <= n6; ++i4) {
                        int n8;
                        if (ahb2.a(n2 + i2, n3 + i3, n4 + i4).o() != awt.j || ((n8 = ahb2.e(n2 + i2, n3 + i3, n4 + i4)) & 8) != 0) continue;
                        ahb2.a(n2 + i2, n3 + i3, n4 + i4, n8 | 8, 4);
                    }
                }
            }
        }
    }
}

