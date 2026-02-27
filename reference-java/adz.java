/*
 * Decompiled with CFR 0.152.
 */
public class adz
extends adb {
    public adz() {
        this.h = 16;
        this.a(abt.c);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 == 0) {
            return false;
        }
        if (!ahb2.a(n2, n3, n4).o().a()) {
            return false;
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
        if (!ajn.an.c(ahb2, n2, n3, n4)) {
            return false;
        }
        if (ahb2.E) {
            return true;
        }
        if (n5 == 1) {
            int n6 = qh.c((double)((yz2.y + 180.0f) * 16.0f / 360.0f) + 0.5) & 0xF;
            ahb2.d(n2, n3, n4, ajn.an, n6, 3);
        } else {
            ahb2.d(n2, n3, n4, ajn.as, n5, 3);
        }
        --add2.b;
        apm apm2 = (apm)ahb2.o(n2, n3, n4);
        if (apm2 != null) {
            yz2.a(apm2);
        }
        return true;
    }
}

