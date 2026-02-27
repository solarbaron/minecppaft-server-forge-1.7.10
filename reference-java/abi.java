/*
 * Decompiled with CFR 0.152.
 */
public class abi
extends adb {
    private aji a;

    public abi(aji aji2) {
        this.a = aji2;
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        int n6;
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2 == ajn.aC && (ahb2.e(n2, n3, n4) & 7) < 1) {
            n5 = 1;
        } else if (aji2 != ajn.bd && aji2 != ajn.H && aji2 != ajn.I) {
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
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (add2.b == 0) {
            return false;
        }
        if (ahb2.a(this.a, n2, n3, n4, false, n5, null, add2) && ahb2.d(n2, n3, n4, this.a, n6 = this.a.a(ahb2, n2, n3, n4, n5, f2, f3, f4, 0), 3)) {
            if (ahb2.a(n2, n3, n4) == this.a) {
                this.a.a(ahb2, n2, n3, n4, yz2, add2);
                this.a.e(ahb2, n2, n3, n4, n6);
            }
            ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), this.a.H.b(), (this.a.H.c() + 1.0f) / 2.0f, this.a.H.d() * 0.8f);
            --add2.b;
        }
        return true;
    }
}

