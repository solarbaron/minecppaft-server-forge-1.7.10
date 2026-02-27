/*
 * Decompiled with CFR 0.152.
 */
public class aec
extends abe {
    public aec(aji aji2, aji aji3) {
        super(aji2, aji3);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        int n6;
        int n7;
        if (add2.b == 0) {
            return false;
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2 == ajn.aC && (n7 = (n6 = ahb2.e(n2, n3, n4)) & 7) <= 6 && ahb2.b(this.a.a(ahb2, n2, n3, n4)) && ahb2.a(n2, n3, n4, n7 + 1 | n6 & 0xFFFFFFF8, 2)) {
            ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), this.a.H.b(), (this.a.H.c() + 1.0f) / 2.0f, this.a.H.d() * 0.8f);
            --add2.b;
            return true;
        }
        return super.a(add2, yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
    }
}

