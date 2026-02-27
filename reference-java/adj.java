/*
 * Decompiled with CFR 0.152.
 */
public class adj
extends adb {
    private static final cp b = new adk();
    public int a;

    public adj(int n2) {
        this.h = 1;
        this.a = n2;
        this.a(abt.e);
        akm.a.a(this, b);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (aje.a(ahb2.a(n2, n3, n4))) {
            if (!ahb2.E) {
                xl xl2 = xl.a(ahb2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, this.a);
                if (add2.u()) {
                    xl2.a(add2.s());
                }
                ahb2.d(xl2);
            }
            --add2.b;
            return true;
        }
        return false;
    }
}

