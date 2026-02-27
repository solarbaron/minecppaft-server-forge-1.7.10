/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class adf
extends adb {
    public adf() {
        this.a(abt.i);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2.b() == 11) {
            if (ahb2.E) {
                return true;
            }
            adf.a(yz2, ahb2, n2, n3, n4);
            return true;
        }
        return false;
    }

    public static boolean a(yz yz2, ahb ahb2, int n2, int n3, int n4) {
        su su2 = su.b(ahb2, n2, n3, n4);
        boolean bl2 = false;
        double d2 = 7.0;
        List list = ahb2.a(sw.class, azt.a((double)n2 - d2, (double)n3 - d2, (double)n4 - d2, (double)n2 + d2, (double)n3 + d2, (double)n4 + d2));
        if (list != null) {
            for (sw sw2 : list) {
                if (!sw2.bN() || sw2.bO() != yz2) continue;
                if (su2 == null) {
                    su2 = su.a(ahb2, n2, n3, n4);
                }
                sw2.b((sa)su2, true);
                bl2 = true;
            }
        }
        return bl2;
    }
}

