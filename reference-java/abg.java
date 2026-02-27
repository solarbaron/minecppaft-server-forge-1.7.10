/*
 * Decompiled with CFR 0.152.
 */
public class abg
extends adb {
    public abg() {
        this.a(abt.c);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        if (n5 != 1) {
            return false;
        }
        ++n3;
        ajh ajh2 = (ajh)ajn.C;
        int n6 = qh.c((double)(yz2.y * 4.0f / 360.0f) + 0.5) & 3;
        int n7 = 0;
        int n8 = 0;
        if (n6 == 0) {
            n8 = 1;
        }
        if (n6 == 1) {
            n7 = -1;
        }
        if (n6 == 2) {
            n8 = -1;
        }
        if (n6 == 3) {
            n7 = 1;
        }
        if (!yz2.a(n2, n3, n4, n5, add2) || !yz2.a(n2 + n7, n3, n4 + n8, n5, add2)) {
            return false;
        }
        if (ahb2.c(n2, n3, n4) && ahb2.c(n2 + n7, n3, n4 + n8) && ahb.a(ahb2, n2, n3 - 1, n4) && ahb.a(ahb2, n2 + n7, n3 - 1, n4 + n8)) {
            ahb2.d(n2, n3, n4, ajh2, n6, 3);
            if (ahb2.a(n2, n3, n4) == ajh2) {
                ahb2.d(n2 + n7, n3, n4 + n8, ajh2, n6 + 8, 3);
            }
            --add2.b;
            return true;
        }
        return false;
    }
}

