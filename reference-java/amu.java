/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class amu
extends ajd {
    private amv a;

    protected amu(String string, awt awt2, amv amv2) {
        super(string, awt2);
        this.a = amv2;
    }

    @Override
    protected int d(int n2) {
        return n2 > 0 ? 1 : 0;
    }

    @Override
    protected int c(int n2) {
        return n2 == 1 ? 15 : 0;
    }

    @Override
    protected int e(ahb ahb2, int n2, int n3, int n4) {
        List list = null;
        if (this.a == amv.a) {
            list = ahb2.b(null, this.a(n2, n3, n4));
        }
        if (this.a == amv.b) {
            list = ahb2.a(sv.class, this.a(n2, n3, n4));
        }
        if (this.a == amv.c) {
            list = ahb2.a(yz.class, this.a(n2, n3, n4));
        }
        if (list != null && !list.isEmpty()) {
            for (sa sa2 : list) {
                if (sa2.az()) continue;
                return 15;
            }
        }
        return 0;
    }
}

