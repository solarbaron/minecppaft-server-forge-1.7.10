/*
 * Decompiled with CFR 0.152.
 */
final class kx
extends cm {
    kx() {
    }

    @Override
    public add b(ck ck2, add add2) {
        cr cr2 = akm.b(ck2.h());
        double d2 = ck2.a() + (double)cr2.c();
        double d3 = (float)ck2.e() + 0.2f;
        double d4 = ck2.c() + (double)cr2.e();
        sa sa2 = aee.a(ck2.k(), add2.k(), d2, d3, d4);
        if (sa2 instanceof sv && add2.u()) {
            ((sw)sa2).a(add2.s());
        }
        add2.a(1);
        return add2;
    }
}

