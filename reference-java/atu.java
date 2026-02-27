/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atu
extends ats {
    public atu() {
    }

    public atu(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((atw)avk2, list, random, 2, 0, false);
        this.b((atw)avk2, list, random, 0, 2, false);
        this.c((atw)avk2, list, random, 0, 2, false);
    }

    public static atu a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -2, 0, 0, 7, 9, 7, n5);
        if (!atu.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atu(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        this.a(ahb2, asv2, 0, 0, 0, 6, 1, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 6, 7, 6, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 2, 0, 1, 6, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 6, 1, 6, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 2, 0, 6, 6, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 2, 6, 6, 6, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 0, 6, 1, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 5, 0, 6, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 2, 0, 6, 6, 1, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 2, 5, 6, 6, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 6, 0, 4, 6, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 5, 0, 4, 5, 0, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 2, 6, 6, 4, 6, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 5, 6, 4, 5, 6, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 6, 2, 0, 6, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 5, 2, 0, 5, 4, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 6, 6, 2, 6, 6, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 5, 2, 6, 5, 4, ajn.bk, ajn.bk, false);
        for (int i2 = 0; i2 <= 6; ++i2) {
            for (int i3 = 0; i3 <= 6; ++i3) {
                this.b(ahb2, ajn.bj, 0, i2, -1, i3, asv2);
            }
        }
        return true;
    }
}

