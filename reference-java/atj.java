/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atj
extends ats {
    public atj() {
    }

    public atj(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((atw)avk2, list, random, 1, 0, true);
    }

    public static atj a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -7, 0, 5, 14, 10, n5);
        if (!atj.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atj(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2 = this.a(ajn.bl, 2);
        for (int i2 = 0; i2 <= 9; ++i2) {
            int n3 = Math.max(1, 7 - i2);
            int n4 = Math.min(Math.max(n3 + 5, 14 - i2), 13);
            int n5 = i2;
            this.a(ahb2, asv2, 0, 0, n5, 4, n3, n5, ajn.bj, ajn.bj, false);
            this.a(ahb2, asv2, 1, n3 + 1, n5, 3, n4 - 1, n5, ajn.a, ajn.a, false);
            if (i2 <= 6) {
                this.a(ahb2, ajn.bl, n2, 1, n3 + 1, n5, asv2);
                this.a(ahb2, ajn.bl, n2, 2, n3 + 1, n5, asv2);
                this.a(ahb2, ajn.bl, n2, 3, n3 + 1, n5, asv2);
            }
            this.a(ahb2, asv2, 0, n4, n5, 4, n4, n5, ajn.bj, ajn.bj, false);
            this.a(ahb2, asv2, 0, n3 + 1, n5, 0, n4 - 1, n5, ajn.bj, ajn.bj, false);
            this.a(ahb2, asv2, 4, n3 + 1, n5, 4, n4 - 1, n5, ajn.bj, ajn.bj, false);
            if ((i2 & 1) == 0) {
                this.a(ahb2, asv2, 0, n3 + 2, n5, 0, n3 + 3, n5, ajn.bk, ajn.bk, false);
                this.a(ahb2, asv2, 4, n3 + 2, n5, 4, n3 + 3, n5, ajn.bk, ajn.bk, false);
            }
            for (int i3 = 0; i3 <= 4; ++i3) {
                this.b(ahb2, ajn.bj, 0, i3, -1, n5, asv2);
            }
        }
        return true;
    }
}

