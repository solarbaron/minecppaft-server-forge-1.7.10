/*
 * Decompiled with CFR 0.152.
 */
public class abh
extends adb {
    protected final aji a;

    public abh(aji aji2) {
        this.a = aji2;
    }

    public abh b(String string) {
        super.c(string);
        return this;
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
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
        if (add2.b == 0) {
            return false;
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (n3 == 255 && this.a.o().a()) {
            return false;
        }
        if (ahb2.a(this.a, n2, n3, n4, false, n5, yz2, add2)) {
            int n6 = this.a(add2.k());
            int n7 = this.a.a(ahb2, n2, n3, n4, n5, f2, f3, f4, n6);
            if (ahb2.d(n2, n3, n4, this.a, n7, 3)) {
                if (ahb2.a(n2, n3, n4) == this.a) {
                    this.a.a(ahb2, n2, n3, n4, yz2, add2);
                    this.a.e(ahb2, n2, n3, n4, n7);
                }
                ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), this.a.H.b(), (this.a.H.c() + 1.0f) / 2.0f, this.a.H.d() * 0.8f);
                --add2.b;
            }
            return true;
        }
        return false;
    }

    @Override
    public String a(add add2) {
        return this.a.a();
    }

    @Override
    public String a() {
        return this.a.a();
    }

    @Override
    public /* synthetic */ adb c(String string) {
        return this.b(string);
    }
}

