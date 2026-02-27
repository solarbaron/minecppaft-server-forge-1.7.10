/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atk
extends ats {
    public atk() {
    }

    public atk(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        int n2 = 1;
        if (this.g == 1 || this.g == 2) {
            n2 = 5;
        }
        this.b((atw)avk2, list, random, 0, n2, random.nextInt(8) > 0);
        this.c((atw)avk2, list, random, 0, n2, random.nextInt(8) > 0);
    }

    public static atk a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -3, 0, 0, 9, 7, 9, n5);
        if (!atk.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atk(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        this.a(ahb2, asv2, 0, 0, 0, 8, 1, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 8, 5, 8, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 6, 0, 8, 6, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 2, 5, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 2, 0, 8, 5, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 3, 0, 1, 4, 0, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 7, 3, 0, 7, 4, 0, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 2, 4, 8, 2, 8, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 1, 4, 2, 2, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 6, 1, 4, 7, 2, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 3, 8, 8, 3, 8, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 6, 0, 3, 7, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 8, 3, 6, 8, 3, 7, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 4, 0, 5, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 8, 3, 4, 8, 5, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 3, 5, 2, 5, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 6, 3, 5, 7, 5, 5, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 4, 5, 1, 5, 5, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 7, 4, 5, 7, 5, 5, ajn.bk, ajn.bk, false);
        for (int i2 = 0; i2 <= 5; ++i2) {
            for (int i3 = 0; i3 <= 8; ++i3) {
                this.b(ahb2, ajn.bj, 0, i3, -1, i2, asv2);
            }
        }
        return true;
    }
}

