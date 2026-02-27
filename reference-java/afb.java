/*
 * Decompiled with CFR 0.152.
 */
public class afb
implements afg {
    @Override
    public boolean a(aae aae2, ahb ahb2) {
        int n2 = 0;
        add add2 = null;
        for (int i2 = 0; i2 < aae2.a(); ++i2) {
            add add3 = aae2.a(i2);
            if (add3 == null) continue;
            if (add3.b() == ade.aY) {
                if (add2 != null) {
                    return false;
                }
                add2 = add3;
                continue;
            }
            if (add3.b() == ade.bJ) {
                ++n2;
                continue;
            }
            return false;
        }
        return add2 != null && n2 > 0;
    }

    @Override
    public add a(aae aae2) {
        int n2 = 0;
        add add2 = null;
        for (int i2 = 0; i2 < aae2.a(); ++i2) {
            add add3 = aae2.a(i2);
            if (add3 == null) continue;
            if (add3.b() == ade.aY) {
                if (add2 != null) {
                    return null;
                }
                add2 = add3;
                continue;
            }
            if (add3.b() == ade.bJ) {
                ++n2;
                continue;
            }
            return null;
        }
        if (add2 == null || n2 < 1) {
            return null;
        }
        add add4 = new add(ade.aY, n2 + 1, add2.k());
        if (add2.u()) {
            add4.c(add2.s());
        }
        return add4;
    }

    @Override
    public int a() {
        return 9;
    }

    @Override
    public add b() {
        return null;
    }
}

