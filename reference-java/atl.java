/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atl
extends ats {
    public atl() {
    }

    public atl(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((atw)avk2, list, random, 5, 3, true);
    }

    public static atl a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -5, -3, 0, 13, 14, 13, n5);
        if (!atl.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atl(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        this.a(ahb2, asv2, 0, 3, 0, 12, 4, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 5, 0, 12, 13, 12, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 5, 0, 1, 12, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 11, 5, 0, 12, 12, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 5, 11, 4, 12, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 8, 5, 11, 10, 12, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 9, 11, 7, 12, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 5, 0, 4, 12, 1, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 8, 5, 0, 10, 12, 1, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 9, 0, 7, 12, 1, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 2, 11, 2, 10, 12, 10, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 5, 8, 0, 7, 8, 0, ajn.bk, ajn.bk, false);
        for (n3 = 1; n3 <= 11; n3 += 2) {
            this.a(ahb2, asv2, n3, 10, 0, n3, 11, 0, ajn.bk, ajn.bk, false);
            this.a(ahb2, asv2, n3, 10, 12, n3, 11, 12, ajn.bk, ajn.bk, false);
            this.a(ahb2, asv2, 0, 10, n3, 0, 11, n3, ajn.bk, ajn.bk, false);
            this.a(ahb2, asv2, 12, 10, n3, 12, 11, n3, ajn.bk, ajn.bk, false);
            this.a(ahb2, ajn.bj, 0, n3, 13, 0, asv2);
            this.a(ahb2, ajn.bj, 0, n3, 13, 12, asv2);
            this.a(ahb2, ajn.bj, 0, 0, 13, n3, asv2);
            this.a(ahb2, ajn.bj, 0, 12, 13, n3, asv2);
            this.a(ahb2, ajn.bk, 0, n3 + 1, 13, 0, asv2);
            this.a(ahb2, ajn.bk, 0, n3 + 1, 13, 12, asv2);
            this.a(ahb2, ajn.bk, 0, 0, 13, n3 + 1, asv2);
            this.a(ahb2, ajn.bk, 0, 12, 13, n3 + 1, asv2);
        }
        this.a(ahb2, ajn.bk, 0, 0, 13, 0, asv2);
        this.a(ahb2, ajn.bk, 0, 0, 13, 12, asv2);
        this.a(ahb2, ajn.bk, 0, 0, 13, 0, asv2);
        this.a(ahb2, ajn.bk, 0, 12, 13, 0, asv2);
        for (n3 = 3; n3 <= 9; n3 += 2) {
            this.a(ahb2, asv2, 1, 7, n3, 1, 8, n3, ajn.bk, ajn.bk, false);
            this.a(ahb2, asv2, 11, 7, n3, 11, 8, n3, ajn.bk, ajn.bk, false);
        }
        this.a(ahb2, asv2, 4, 2, 0, 8, 2, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 4, 12, 2, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 4, 0, 0, 8, 1, 3, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 4, 0, 9, 8, 1, 12, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 0, 4, 3, 1, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 9, 0, 4, 12, 1, 8, ajn.bj, ajn.bj, false);
        for (n3 = 4; n3 <= 8; ++n3) {
            for (n2 = 0; n2 <= 2; ++n2) {
                this.b(ahb2, ajn.bj, 0, n3, -1, n2, asv2);
                this.b(ahb2, ajn.bj, 0, n3, -1, 12 - n2, asv2);
            }
        }
        for (n3 = 0; n3 <= 2; ++n3) {
            for (n2 = 4; n2 <= 8; ++n2) {
                this.b(ahb2, ajn.bj, 0, n3, -1, n2, asv2);
                this.b(ahb2, ajn.bj, 0, 12 - n3, -1, n2, asv2);
            }
        }
        this.a(ahb2, asv2, 5, 5, 5, 7, 5, 7, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 1, 6, 6, 4, 6, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.bj, 0, 6, 0, 6, asv2);
        this.a(ahb2, ajn.k, 0, 6, 5, 6, asv2);
        n3 = this.a(6, 6);
        n2 = this.a(5);
        int n4 = this.b(6, 6);
        if (asv2.b(n3, n2, n4)) {
            ahb2.d = true;
            ajn.k.a(ahb2, n3, n2, n4, random);
            ahb2.d = false;
        }
        return true;
    }
}

