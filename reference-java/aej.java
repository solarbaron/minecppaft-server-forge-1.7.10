/*
 * Decompiled with CFR 0.152.
 */
public class aej
extends abr {
    public aej(aji aji2) {
        super(aji2, false);
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
            if (ahb2.a(n2, n3, n4).o() == awt.h && ahb2.e(n2, n3, n4) == 0 && ahb2.c(n2, n3 + 1, n4)) {
                ahb2.b(n2, n3 + 1, n4, ajn.bi);
                if (!yz2.bE.d) {
                    --add2.b;
                }
            }
        }
        return add2;
    }
}

