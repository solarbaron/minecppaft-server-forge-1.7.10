/*
 * Decompiled with CFR 0.152.
 */
public class acl
extends abs {
    protected acl() {
        this.a(abt.f);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        add add3 = new add(ade.aY, 1, ahb2.b("map"));
        String string = "map_" + add3.k();
        ayi ayi2 = new ayi(string);
        ahb2.a(string, ayi2);
        ayi2.d = 0;
        int n2 = 128 * (1 << ayi2.d);
        ayi2.a = (int)(Math.round(yz2.s / (double)n2) * (long)n2);
        ayi2.b = (int)(Math.round(yz2.u / (double)n2) * (long)n2);
        ayi2.c = (byte)ahb2.t.i;
        ayi2.c();
        --add2.b;
        if (add2.b <= 0) {
            return add3;
        }
        if (!yz2.bm.a(add3.m())) {
            yz2.a(add3, false);
        }
        return add2;
    }
}

