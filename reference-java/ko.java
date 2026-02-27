/*
 * Decompiled with CFR 0.152.
 */
final class ko
extends cm {
    private final cm b = new cm();

    ko() {
    }

    @Override
    public add b(ck ck2, add add2) {
        adb adb2;
        cr cr2 = akm.b(ck2.h());
        ahb ahb2 = ck2.k();
        int n2 = ck2.d() + cr2.c();
        int n3 = ck2.e() + cr2.d();
        int n4 = ck2.f() + cr2.e();
        awt awt2 = ahb2.a(n2, n3, n4).o();
        int n5 = ahb2.e(n2, n3, n4);
        if (awt.h.equals(awt2) && n5 == 0) {
            adb2 = ade.as;
        } else if (awt.i.equals(awt2) && n5 == 0) {
            adb2 = ade.at;
        } else {
            return super.b(ck2, add2);
        }
        ahb2.f(n2, n3, n4);
        if (--add2.b == 0) {
            add2.a(adb2);
            add2.b = 1;
        } else if (((apb)ck2.j()).a(new add(adb2)) < 0) {
            this.b.a(ck2, new add(adb2));
        }
        return add2;
    }
}

