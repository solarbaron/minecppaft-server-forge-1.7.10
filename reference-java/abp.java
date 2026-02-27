/*
 * Decompiled with CFR 0.152.
 */
public class abp
extends adb {
    public abp() {
        this.a(abt.e);
        this.e(1);
        this.f(25);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        wo wo2;
        if (yz2.am() && yz2.m instanceof wo && (wo2 = (wo)yz2.m).ca().h() && add2.l() - add2.k() >= 7) {
            wo2.ca().g();
            add2.a(7, (sv)yz2);
            if (add2.b == 0) {
                add add3 = new add(ade.aM);
                add3.d(add2.d);
                return add3;
            }
        }
        return add2;
    }
}

