/*
 * Decompiled with CFR 0.152.
 */
public class aee
extends adb {
    public aee() {
        this.a(true);
        this.a(abt.f);
    }

    @Override
    public String n(add add2) {
        String string = ("" + dd.a(this.a() + ".name")).trim();
        String string2 = sg.b(add2.k());
        if (string2 != null) {
            string = string + " " + dd.a("entity." + string2 + ".name");
        }
        return string;
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        sa sa2;
        if (ahb2.E) {
            return true;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        n2 += q.b[n5];
        n3 += q.c[n5];
        n4 += q.d[n5];
        double d2 = 0.0;
        if (n5 == 1 && aji2.b() == 11) {
            d2 = 0.5;
        }
        if ((sa2 = aee.a(ahb2, add2.k(), (double)n2 + 0.5, (double)n3 + d2, (double)n4 + 0.5)) != null) {
            if (sa2 instanceof sv && add2.u()) {
                ((sw)sa2).a(add2.s());
            }
            if (!yz2.bE.d) {
                --add2.b;
            }
        }
        return true;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (ahb2.E) {
            return add2;
        }
        azu azu2 = this.a(ahb2, yz2, true);
        if (azu2 == null) {
            return add2;
        }
        if (azu2.a == azv.b) {
            sa sa2;
            int n2 = azu2.b;
            int n3 = azu2.c;
            int n4 = azu2.d;
            if (!ahb2.a(yz2, n2, n3, n4)) {
                return add2;
            }
            if (!yz2.a(n2, n3, n4, azu2.e, add2)) {
                return add2;
            }
            if (ahb2.a(n2, n3, n4) instanceof alw && (sa2 = aee.a(ahb2, add2.k(), n2, n3, n4)) != null) {
                if (sa2 instanceof sv && add2.u()) {
                    ((sw)sa2).a(add2.s());
                }
                if (!yz2.bE.d) {
                    --add2.b;
                }
            }
        }
        return add2;
    }

    public static sa a(ahb ahb2, int n2, double d2, double d3, double d4) {
        if (!sg.a.containsKey(n2)) {
            return null;
        }
        sa sa2 = null;
        for (int i2 = 0; i2 < 1; ++i2) {
            sa2 = sg.a(n2, ahb2);
            if (sa2 == null || !(sa2 instanceof sv)) continue;
            sw sw2 = (sw)sa2;
            sa2.b(d2, d3, d4, qh.g(ahb2.s.nextFloat() * 360.0f), 0.0f);
            sw2.aO = sw2.y;
            sw2.aM = sw2.y;
            sw2.a((sy)null);
            ahb2.d(sa2);
            sw2.r();
        }
        return sa2;
    }
}

