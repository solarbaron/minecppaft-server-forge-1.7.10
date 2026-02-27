/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atp
extends ats {
    private boolean b;

    public atp() {
    }

    public atp(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
        this.b = random.nextInt(3) == 0;
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.n("Chest");
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Chest", this.b);
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.c((atw)avk2, list, random, 0, 1, true);
    }

    public static atp a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, 0, 0, 5, 7, 5, n5);
        if (!atp.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new atp(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        this.a(ahb2, asv2, 0, 0, 0, 4, 1, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 2, 0, 4, 5, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 2, 0, 0, 5, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 0, 3, 1, 0, 4, 1, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 0, 3, 3, 0, 4, 3, ajn.bk, ajn.bk, false);
        this.a(ahb2, asv2, 4, 2, 0, 4, 5, 0, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 2, 4, 4, 5, 4, ajn.bj, ajn.bj, false);
        this.a(ahb2, asv2, 1, 3, 4, 1, 4, 4, ajn.bk, ajn.bj, false);
        this.a(ahb2, asv2, 3, 3, 4, 3, 4, 4, ajn.bk, ajn.bj, false);
        if (this.b) {
            int n4;
            n3 = this.a(2);
            n2 = this.a(1, 3);
            if (asv2.b(n2, n3, n4 = this.b(1, 3))) {
                this.b = false;
                this.a(ahb2, asv2, random, 1, 2, 3, a, 2 + random.nextInt(4));
            }
        }
        this.a(ahb2, asv2, 0, 6, 0, 4, 6, 4, ajn.bj, ajn.bj, false);
        for (n3 = 0; n3 <= 4; ++n3) {
            for (n2 = 0; n2 <= 4; ++n2) {
                this.b(ahb2, ajn.bj, 0, n3, -1, n2, asv2);
            }
        }
        return true;
    }
}

