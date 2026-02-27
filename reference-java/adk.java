/*
 * Decompiled with CFR 0.152.
 */
final class adk
extends cm {
    private final cm b = new cm();

    adk() {
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
        aji aji2 = ahb2.a(n4, n3 = ck2.e() + cr2.d(), n2 = ck2.f() + cr2.e());
        if (aje.a(aji2)) {
            d2 = 0.0;
        } else if (aji2.o() == awt.a && aje.a(ahb2.a(n4, n3 - 1, n2))) {
            d2 = -1.0;
        } else {
            return this.b.a(ck2, add2);
        }
        xl xl2 = xl.a(ahb2, d3, d4 + d2, d5, ((adj)add2.b()).a);
        if (add2.u()) {
            xl2.a(add2.s());
        }
        ahb2.d(xl2);
        add2.a(1);
        return add2;
    }

    @Override
    protected void a(ck ck2) {
        ck2.k().c(1000, ck2.d(), ck2.e(), ck2.f(), 0);
    }
}

