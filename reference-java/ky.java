/*
 * Decompiled with CFR 0.152.
 */
final class ky
extends cm {
    ky() {
    }

    @Override
    public add b(ck ck2, add add2) {
        cr cr2 = akm.b(ck2.h());
        double d2 = ck2.a() + (double)cr2.c();
        double d3 = (float)ck2.e() + 0.2f;
        double d4 = ck2.c() + (double)cr2.e();
        zf zf2 = new zf(ck2.k(), d2, d3, d4, add2);
        ck2.k().d(zf2);
        add2.a(1);
        return add2;
    }

    @Override
    protected void a(ck ck2) {
        ck2.k().c(1002, ck2.d(), ck2.e(), ck2.f(), 0);
    }
}

