/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class arp
extends asq {
    private int a;
    private int b;

    public arp(int n2, int n3) {
        super(false);
        this.b = n2;
        this.a = n3;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        aji aji2;
        while (((aji2 = ahb2.a(n2, n3, n4)).o() == awt.a || aji2.o() == awt.j) && n3 > 0) {
            --n3;
        }
        aji aji3 = ahb2.a(n2, n3, n4);
        if (aji3 == ajn.d || aji3 == ajn.c) {
            this.a(ahb2, n2, ++n3, n4, ajn.r, this.b);
            for (int i2 = n3; i2 <= n3 + 2; ++i2) {
                int n5 = i2 - n3;
                int n6 = 2 - n5;
                for (int i3 = n2 - n6; i3 <= n2 + n6; ++i3) {
                    int n7 = i3 - n2;
                    for (int i4 = n4 - n6; i4 <= n4 + n6; ++i4) {
                        int n8 = i4 - n4;
                        if (Math.abs(n7) == n6 && Math.abs(n8) == n6 && random.nextInt(2) == 0 || ahb2.a(i3, i2, i4).j()) continue;
                        this.a(ahb2, i3, i2, i4, ajn.t, this.a);
                    }
                }
            }
        }
        return true;
    }
}

