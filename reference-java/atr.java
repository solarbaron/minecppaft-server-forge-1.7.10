/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atr
extends ats {
    private boolean b;

    public atr() {
    }

    public atr(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.n("Mob");
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Mob", this.b);
    }

    public static atr a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -2, 0, 0, 7, 8, 9, n5);
        if (!atr.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atr(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        this.a(ahb2, asv2, 0, 2, 0, 6, 7, 7, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 0, 0, 5, 1, 7, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 2, 1, 5, 2, 7, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 3, 2, 5, 3, 7, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 4, 3, 5, 4, 7, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 2, 0, 1, 4, 2, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 2, 0, 5, 4, 2, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 5, 2, 1, 5, 3, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 5, 2, 5, 5, 3, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 5, 3, 0, 5, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 5, 3, 6, 5, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 5, 8, 5, 5, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, ajn.bk, 0, 1, 6, 3, asv2);
        this.a(ahb2, ajn.bk, 0, 5, 6, 3, asv2);
        this.a(ahb2, asv2, 0, 6, 3, 0, 6, 8, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 6, 6, 3, 6, 6, 8, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 1, 6, 8, 5, 7, 8, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 2, 8, 8, 4, 8, 8, ajn.bk, ajn.bk, false);
        if (!this.b) {
            int n4;
            n3 = this.a(5);
            n2 = this.a(3, 5);
            if (asv2.b(n2, n3, n4 = this.b(3, 5))) {
                this.b = true;
                ahb2.d(n2, n3, n4, ajn.ac, 0, 2);
                apj apj2 = (apj)ahb2.o(n2, n3, n4);
                if (apj2 != null) {
                    apj2.a().a("Blaze");
                }
            }
        }
        for (n3 = 0; n3 <= 6; ++n3) {
            for (n2 = 0; n2 <= 6; ++n2) {
                this.b(ahb2, ajn.bj, 0, n3, -1, n2, asv2);
            }
        }
        return true;
    }
}

