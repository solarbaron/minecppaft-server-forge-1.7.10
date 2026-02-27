/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amn
extends aji {
    public amn() {
        super(awt.e);
        this.a(abt.b);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (this == ajn.q) {
            return ade.h;
        }
        if (this == ajn.ag) {
            return ade.i;
        }
        if (this == ajn.x) {
            return ade.aR;
        }
        if (this == ajn.bA) {
            return ade.bC;
        }
        if (this == ajn.bY) {
            return ade.bU;
        }
        return adb.a(this);
    }

    @Override
    public int a(Random random) {
        if (this == ajn.x) {
            return 4 + random.nextInt(5);
        }
        return 1;
    }

    @Override
    public int a(int n2, Random random) {
        if (n2 > 0 && adb.a(this) != this.a(0, random, n2)) {
            int n3 = random.nextInt(n2 + 2) - 1;
            if (n3 < 0) {
                n3 = 0;
            }
            return this.a(random) * (n3 + 1);
        }
        return this.a(random);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, n6);
        if (this.a(n5, ahb2.s, n6) != adb.a(this)) {
            int n7 = 0;
            if (this == ajn.q) {
                n7 = qh.a(ahb2.s, 0, 2);
            } else if (this == ajn.ag) {
                n7 = qh.a(ahb2.s, 3, 7);
            } else if (this == ajn.bA) {
                n7 = qh.a(ahb2.s, 3, 7);
            } else if (this == ajn.x) {
                n7 = qh.a(ahb2.s, 2, 5);
            } else if (this == ajn.bY) {
                n7 = qh.a(ahb2.s, 2, 5);
            }
            this.c(ahb2, n2, n3, n4, n7);
        }
    }

    @Override
    public int a(int n2) {
        if (this == ajn.x) {
            return 4;
        }
        return 0;
    }
}

