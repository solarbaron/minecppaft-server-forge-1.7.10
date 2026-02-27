/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

final class abc
extends cm {
    abc() {
    }

    @Override
    protected add b(ck ck2, add add2) {
        cr cr2 = akm.b(ck2.h());
        int n2 = ck2.d() + cr2.c();
        int n3 = ck2.e() + cr2.d();
        int n4 = ck2.f() + cr2.e();
        azt azt2 = azt.a(n2, n3, n4, n2 + 1, n3 + 1, n4 + 1);
        List list = ck2.k().a(sv.class, azt2, (sj)new sp(add2));
        if (list.size() > 0) {
            sv sv2 = (sv)list.get(0);
            int n5 = sv2 instanceof yz ? 1 : 0;
            int n6 = sw.b(add2);
            add add3 = add2.m();
            add3.b = 1;
            sv2.c(n6 - n5, add3);
            if (sv2 instanceof sw) {
                ((sw)sv2).a(n6, 2.0f);
            }
            --add2.b;
            return add2;
        }
        return super.b(ck2, add2);
    }
}

