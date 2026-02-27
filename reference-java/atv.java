/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atv
extends ats {
    public atv() {
    }

    public atv(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.c((atw)avk2, list, random, 6, 2, false);
    }

    public static atv a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -2, 0, 0, 7, 11, 7, n5);
        if (!atv.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atv(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        this.a(ahb2, asv2, 0, 0, 0, 6, 1, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 6, 10, 6, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 2, 0, 1, 8, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 2, 0, 6, 8, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 1, 0, 8, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 2, 1, 6, 8, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 2, 6, 5, 8, 6, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 3, 2, 0, 5, 4, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 6, 3, 2, 6, 5, 2, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 6, 3, 4, 6, 5, 4, ajn.bk, ajn.bk, false);
        this.a(ahb2, ajn.bj, 0, 5, 2, 5, asv2);
        this.a(ahb2, asv2, 4, 2, 5, 4, 3, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 3, 2, 5, 3, 4, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 2, 5, 2, 5, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 2, 5, 1, 6, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 7, 1, 5, 7, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 8, 2, 6, 8, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 2, 6, 0, 4, 8, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 5, 0, 4, 5, 0, ajn.bk, ajn.bk, false);
        for (int i2 = 0; i2 <= 6; ++i2) {
            for (int i3 = 0; i3 <= 6; ++i3) {
                this.b(ahb2, ajn.bj, 0, i2, -1, i3, asv2);
            }
        }
        return true;
    }
}

