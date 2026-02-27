/*
 * Decompiled with CFR 0.152.
 */
final class kp
extends cm {
    private boolean b = true;

    kp() {
    }

    @Override
    protected add b(ck ck2, add add2) {
        int n2;
        int n3;
        int n4;
        cr cr2 = akm.b(ck2.h());
        ahb ahb2 = ck2.k();
        if (ahb2.c(n4 = ck2.d() + cr2.c(), n3 = ck2.e() + cr2.d(), n2 = ck2.f() + cr2.e())) {
            ahb2.b(n4, n3, n2, ajn.ab);
            if (add2.a(1, ahb2.s)) {
                add2.b = 0;
            }
        } else if (ahb2.a(n4, n3, n2) == ajn.W) {
            ajn.W.b(ahb2, n4, n3, n2, 1);
            ahb2.f(n4, n3, n2);
        } else {
            this.b = false;
        }
        return add2;
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

