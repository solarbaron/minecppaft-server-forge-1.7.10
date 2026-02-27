/*
 * Decompiled with CFR 0.152.
 */
final class la
extends cm {
    private final cm b = new cm();

    la() {
    }

    @Override
    public add b(ck ck2, add add2) {
        double d2;
        int n2;
        int n3;
        cr cr2 = akm.b(ck2.h());
        ahb ahb2 = ck2.k();
        double d3 = ck2.a() + (double)((float)cr2.c() * 1.125f);
        double d4 = ck2.b() + (double)((float)cr2.d() * 1.125f);
        double d5 = ck2.c() + (double)((float)cr2.e() * 1.125f);
        int n4 = ck2.d() + cr2.c();
        awt awt2 = ahb2.a(n4, n3 = ck2.e() + cr2.d(), n2 = ck2.f() + cr2.e()).o();
        if (awt.h.equals(awt2)) {
            d2 = 1.0;
        } else if (awt.a.equals(awt2) && awt.h.equals(ahb2.a(n4, n3 - 1, n2).o())) {
            d2 = 0.0;
        } else {
            return this.b.a(ck2, add2);
        }
        xi xi2 = new xi(ahb2, d3, d4 + d2, d5);
        ahb2.d(xi2);
        add2.a(1);
        return add2;
    }

    @Override
    protected void a(ck ck2) {
        ck2.k().c(1000, ck2.d(), ck2.e(), ck2.f(), 0);
    }
}

