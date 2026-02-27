/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.Map;

public class adr
extends adb {
    private static final Map b = new HashMap();
    public final String a;

    protected adr(String string) {
        this.a = string;
        this.h = 1;
        this.a(abt.f);
        b.put(string, this);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (ahb2.a(n2, n3, n4) == ajn.aI && ahb2.e(n2, n3, n4) == 0) {
            if (ahb2.E) {
                return true;
            }
            ((alq)ajn.aI).b(ahb2, n2, n3, n4, add2);
            ahb2.a(null, 1005, n2, n3, n4, adb.b(this));
            --add2.b;
            return true;
        }
        return false;
    }

    @Override
    public adq f(add add2) {
        return adq.c;
    }
}

