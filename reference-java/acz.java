/*
 * Decompiled with CFR 0.152.
 */
public class acz
extends adb {
    private final Class a;

    public acz(Class clazz) {
        this.a = clazz;
        this.a(abt.c);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 == 0) {
            return false;
        }
        if (n5 == 1) {
            return false;
        }
        int n6 = p.e[n5];
        ss ss2 = this.a(ahb2, n2, n3, n4, n6);
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (ss2 != null && ss2.e()) {
            if (!ahb2.E) {
                ahb2.d(ss2);
            }
            --add2.b;
        }
        return true;
    }

    private ss a(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (this.a == tb.class) {
            return new tb(ahb2, n2, n3, n4, n5);
        }
        if (this.a == st.class) {
            return new st(ahb2, n2, n3, n4, n5);
        }
        return null;
    }
}

