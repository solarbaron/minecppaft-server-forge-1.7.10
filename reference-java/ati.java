/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ati
extends ats {
    public ati() {
    }

    public ati(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((atw)avk2, list, random, 1, 3, false);
    }

    public static ati a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -3, 0, 5, 10, 19, n5);
        if (!ati.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new ati(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        this.a(ahb2, asv2, 0, 3, 0, 4, 4, 18, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 5, 0, 3, 7, 18, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 5, 0, 0, 5, 18, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 4, 5, 0, 4, 5, 18, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 4, 2, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 13, 4, 2, 18, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 0, 0, 4, 1, 3, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 0, 15, 4, 1, 18, ajn.bj, ajn.bj, false);
        for (int i2 = 0; i2 <= 4; ++i2) {
            for (int i3 = 0; i3 <= 2; ++i3) {
                this.b(ahb2, ajn.bj, 0, i2, -1, i3, asv2);
                this.b(ahb2, ajn.bj, 0, i2, -1, 18 - i3, asv2);
            }
        }
        this.a(ahb2, asv2, 0, 1, 1, 0, 4, 1, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 4, 0, 4, 4, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 14, 0, 4, 14, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 1, 17, 0, 4, 17, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 1, 1, 4, 4, 1, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 3, 4, 4, 4, 4, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 3, 14, 4, 4, 14, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 1, 17, 4, 4, 17, ajn.bk, ajn.bk, false);
        return true;
    }
}

