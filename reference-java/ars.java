/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ars
extends arn {
    private aji a;
    private boolean b;

    public ars(aji aji2, boolean bl2) {
        this.a = aji2;
        this.b = bl2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3 + 1, n4) != ajn.aL) {
            return false;
        }
        if (ahb2.a(n2, n3, n4).o() != awt.a && ahb2.a(n2, n3, n4) != ajn.aL) {
            return false;
        }
        int n5 = 0;
        if (ahb2.a(n2 - 1, n3, n4) == ajn.aL) {
            ++n5;
        }
        if (ahb2.a(n2 + 1, n3, n4) == ajn.aL) {
            ++n5;
        }
        if (ahb2.a(n2, n3, n4 - 1) == ajn.aL) {
            ++n5;
        }
        if (ahb2.a(n2, n3, n4 + 1) == ajn.aL) {
            ++n5;
        }
        if (ahb2.a(n2, n3 - 1, n4) == ajn.aL) {
            ++n5;
        }
        int n6 = 0;
        if (ahb2.c(n2 - 1, n3, n4)) {
            ++n6;
        }
        if (ahb2.c(n2 + 1, n3, n4)) {
            ++n6;
        }
        if (ahb2.c(n2, n3, n4 - 1)) {
            ++n6;
        }
        if (ahb2.c(n2, n3, n4 + 1)) {
            ++n6;
        }
        if (ahb2.c(n2, n3 - 1, n4)) {
            ++n6;
        }
        if (!this.b && n5 == 4 && n6 == 1 || n5 == 5) {
            ahb2.d(n2, n3, n4, this.a, 0, 2);
            ahb2.d = true;
            this.a.a(ahb2, n2, n3, n4, random);
            ahb2.d = false;
        }
        return true;
    }
}

