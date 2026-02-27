/*
 * Decompiled with CFR 0.152.
 */
public class abl
extends adb {
    public abl() {
        this.a(abt.k);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        azu azu2 = this.a(ahb2, yz2, true);
        if (azu2 == null) {
            return add2;
        }
        if (azu2.a == azv.b) {
            int n2 = azu2.b;
            int n3 = azu2.c;
            int n4 = azu2.d;
            if (!ahb2.a(yz2, n2, n3, n4)) {
                return add2;
            }
            if (!yz2.a(n2, n3, n4, azu2.e, add2)) {
                return add2;
            }
            if (ahb2.a(n2, n3, n4).o() == awt.h) {
                --add2.b;
                if (add2.b <= 0) {
                    return new add(ade.bn);
                }
                if (!yz2.bm.a(new add(ade.bn))) {
                    yz2.a(new add(ade.bn, 1, 0), false);
                }
            }
        }
        return add2;
    }
}

