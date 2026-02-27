/*
 * Decompiled with CFR 0.152.
 */
public class ads
extends adb {
    public ads() {
        this.a(abt.d);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (ahb2.a(n2, n3, n4) != ajn.aC) {
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
            if (!ahb2.c(n2, n3, n4)) {
                return false;
            }
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (ajn.af.c(ahb2, n2, n3, n4)) {
            --add2.b;
            ahb2.b(n2, n3, n4, ajn.af);
        }
        return true;
    }
}

