/*
 * Decompiled with CFR 0.152.
 */
final class kq
extends cm {
    private boolean b = true;

    kq() {
    }

    @Override
    protected add b(ck ck2, add add2) {
        if (add2.k() == 15) {
            int n2;
            int n3;
            int n4;
            cr cr2 = akm.b(ck2.h());
            ahb ahb2 = ck2.k();
            if (acj.a(add2, ahb2, n4 = ck2.d() + cr2.c(), n3 = ck2.e() + cr2.d(), n2 = ck2.f() + cr2.e())) {
                if (!ahb2.E) {
                    ahb2.c(2005, n4, n3, n2, 0);
                }
            } else {
                this.b = false;
            }
            return add2;
        }
        return super.b(ck2, add2);
    }

    @Override
    protected void a(ck ck2) {
        if (this.b) {
            ck2.k().c(1000, ck2.d(), ck2.e(), ck2.f(), 0);
        } else {
            ck2.k().c(1001, ck2.d(), ck2.e(), ck2.f(), 0);
        }
    }
}

