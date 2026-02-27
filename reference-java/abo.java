/*
 * Decompiled with CFR 0.152.
 */
public class abo
extends adb {
    private aji a;

    public abo(aji aji2) {
        this.h = 1;
        this.a = aji2;
        this.a(abt.f);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        boolean bl2 = this.a == ajn.a;
        azu azu2 = this.a(ahb2, yz2, bl2);
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
            if (bl2) {
                if (!yz2.a(n2, n3, n4, azu2.e, add2)) {
                    return add2;
                }
                awt awt2 = ahb2.a(n2, n3, n4).o();
                int n5 = ahb2.e(n2, n3, n4);
                if (awt2 == awt.h && n5 == 0) {
                    ahb2.f(n2, n3, n4);
                    return this.a(add2, yz2, ade.as);
                }
                if (awt2 == awt.i && n5 == 0) {
                    ahb2.f(n2, n3, n4);
                    return this.a(add2, yz2, ade.at);
                }
            } else {
                if (this.a == ajn.a) {
                    return new add(ade.ar);
                }
                if (azu2.e == 0) {
                    --n3;
                }
                if (azu2.e == 1) {
                    ++n3;
                }
                if (azu2.e == 2) {
                    --n4;
                }
                if (azu2.e == 3) {
                    ++n4;
                }
                if (azu2.e == 4) {
                    --n2;
                }
                if (azu2.e == 5) {
                    ++n2;
                }
                if (!yz2.a(n2, n3, n4, azu2.e, add2)) {
                    return add2;
                }
                if (this.a(ahb2, n2, n3, n4) && !yz2.bE.d) {
                    return new add(ade.ar);
                }
            }
        }
        return add2;
    }

    private add a(add add2, yz yz2, adb adb2) {
        if (yz2.bE.d) {
            return add2;
        }
        if (--add2.b <= 0) {
            return new add(adb2);
        }
        if (!yz2.bm.a(new add(adb2))) {
            yz2.a(new add(adb2, 1, 0), false);
        }
        return add2;
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4) {
        boolean bl2;
        if (this.a == ajn.a) {
            return false;
        }
        awt awt2 = ahb2.a(n2, n3, n4).o();
        boolean bl3 = bl2 = !awt2.a();
        if (ahb2.c(n2, n3, n4) || bl2) {
            if (ahb2.t.f && this.a == ajn.i) {
                ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), "random.fizz", 0.5f, 2.6f + (ahb2.s.nextFloat() - ahb2.s.nextFloat()) * 0.8f);
                for (int i2 = 0; i2 < 8; ++i2) {
                    ahb2.a("largesmoke", (double)n2 + Math.random(), (double)n3 + Math.random(), (double)n4 + Math.random(), 0.0, 0.0, 0.0);
                }
            } else {
                if (!ahb2.E && bl2 && !awt2.d()) {
                    ahb2.a(n2, n3, n4, true);
                }
                ahb2.d(n2, n3, n4, this.a, 0, 3);
            }
            return true;
        }
        return false;
    }
}

