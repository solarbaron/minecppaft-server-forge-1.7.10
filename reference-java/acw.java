/*
 * Decompiled with CFR 0.152.
 */
public class acw
extends adb {
    public acw() {
        this.h = 1;
        this.f(64);
        this.a(abt.i);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 == 0) {
            --n3;
        }
        if (n5 == 1) {
            ++n3;
        }
        if (n5 == 2) {
            --n4;
        }
        if (n5 == 3) {
            ++n4;
        }
        if (n5 == 4) {
            --n2;
        }
        if (n5 == 5) {
            ++n2;
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (ahb2.a(n2, n3, n4).o() == awt.a) {
            ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "fire.ignite", 1.0f, g.nextFloat() * 0.4f + 0.8f);
            ahb2.b(n2, n3, n4, ajn.ab);
        }
        add2.a(1, (sv)yz2);
        return true;
    }
}

