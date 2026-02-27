/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ato
extends ats {
    public ato() {
    }

    public ato(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((atw)avk2, list, random, 1, 0, true);
    }

    public static ato a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, 0, 0, 5, 7, 5, n5);
        if (!ato.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new ato(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        this.a(ahb2, asv2, 0, 0, 0, 4, 1, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 4, 5, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 2, 0, 0, 5, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 4, 2, 0, 4, 5, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 3, 1, 0, 4, 1, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 3, 0, 4, 3, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 3, 1, 4, 4, 1, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 3, 3, 4, 4, 3, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 6, 0, 4, 6, 4, ajn.bj, ajn.bj, false);
        for (int i2 = 0; i2 <= 4; ++i2) {
            for (int i3 = 0; i3 <= 4; ++i3) {
                this.b(ahb2, ajn.bj, 0, i2, -1, i3, asv2);
            }
        }
        return true;
    }
}

