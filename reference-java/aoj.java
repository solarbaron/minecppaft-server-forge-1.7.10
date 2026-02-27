/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class aoj
extends ajr {
    protected aoj() {
        float f2 = 0.5f;
        float f3 = 0.015625f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f3, 0.5f + f2);
        this.a(abt.c);
    }

    @Override
    public int b() {
        return 23;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        if (sa2 == null || !(sa2 instanceof xi)) {
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (double)n3 + this.F, (double)n4 + this.G);
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2 == ajn.j;
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        if (n3 < 0 || n3 >= 256) {
            return false;
        }
        return ahb2.a(n2, n3 - 1, n4).o() == awt.h && ahb2.e(n2, n3 - 1, n4) == 0;
    }
}

