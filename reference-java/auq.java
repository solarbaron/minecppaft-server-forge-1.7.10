/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auq
extends avc {
    private static final qx[] a = new qx[]{new qx(ade.aG, 0, 1, 3, 20), new qx(ade.aF, 0, 2, 7, 20), new qx(ade.bJ, 0, 1, 1, 1), new qx(ade.aL, 0, 1, 1, 1)};
    private boolean b;

    public auq() {
    }

    public auq(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
        this.b = asv2.c() > 6;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Tall", this.b);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.n("Tall");
    }

    public static auq a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -4, -1, 0, 14, 11, 15, n5);
        if (!(auq.a(asv2) && avk.a(list, asv2) == null || auq.a(asv2 = asv.a(n2, n3, n4, -4, -1, 0, 14, 6, 15, n5)) && avk.a(list, asv2) == null)) {
            return null;
        }
        return new auq(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (this.a(ahb2, asv2)) {
            return false;
        }
        int n3 = 11;
        if (!this.b) {
            n3 = 6;
        }
        this.a(ahb2, asv2, 0, 0, 0, 13, n3 - 1, 14, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 4, 1, 0);
        this.a(ahb2, asv2, random, 0.07f, 2, 1, 1, 11, 4, 13, ajn.G, ajn.G, false);
        boolean bl2 = true;
        int n4 = 12;
        for (n2 = 1; n2 <= 13; ++n2) {
            if ((n2 - 1) % 4 == 0) {
                this.a(ahb2, asv2, 1, 1, n2, 1, 4, n2, ajn.f, ajn.f, false);
                this.a(ahb2, asv2, 12, 1, n2, 12, 4, n2, ajn.f, ajn.f, false);
                this.a(ahb2, ajn.aa, 0, 2, 3, n2, asv2);
                this.a(ahb2, ajn.aa, 0, 11, 3, n2, asv2);
                if (!this.b) continue;
                this.a(ahb2, asv2, 1, 6, n2, 1, 9, n2, ajn.f, ajn.f, false);
                this.a(ahb2, asv2, 12, 6, n2, 12, 9, n2, ajn.f, ajn.f, false);
                continue;
            }
            this.a(ahb2, asv2, 1, 1, n2, 1, 4, n2, ajn.X, ajn.X, false);
            this.a(ahb2, asv2, 12, 1, n2, 12, 4, n2, ajn.X, ajn.X, false);
            if (!this.b) continue;
            this.a(ahb2, asv2, 1, 6, n2, 1, 9, n2, ajn.X, ajn.X, false);
            this.a(ahb2, asv2, 12, 6, n2, 12, 9, n2, ajn.X, ajn.X, false);
        }
        for (n2 = 3; n2 < 12; n2 += 2) {
            this.a(ahb2, asv2, 3, 1, n2, 4, 3, n2, ajn.X, ajn.X, false);
            this.a(ahb2, asv2, 6, 1, n2, 7, 3, n2, ajn.X, ajn.X, false);
            this.a(ahb2, asv2, 9, 1, n2, 10, 3, n2, ajn.X, ajn.X, false);
        }
        if (this.b) {
            this.a(ahb2, asv2, 1, 5, 1, 3, 5, 13, ajn.f, ajn.f, false);
            this.a(ahb2, asv2, 10, 5, 1, 12, 5, 13, ajn.f, ajn.f, false);
            this.a(ahb2, asv2, 4, 5, 1, 9, 5, 2, ajn.f, ajn.f, false);
            this.a(ahb2, asv2, 4, 5, 12, 9, 5, 13, ajn.f, ajn.f, false);
            this.a(ahb2, ajn.f, 0, 9, 5, 11, asv2);
            this.a(ahb2, ajn.f, 0, 8, 5, 11, asv2);
            this.a(ahb2, ajn.f, 0, 9, 5, 10, asv2);
            this.a(ahb2, asv2, 3, 6, 2, 3, 6, 12, ajn.aJ, ajn.aJ, false);
            this.a(ahb2, asv2, 10, 6, 2, 10, 6, 10, ajn.aJ, ajn.aJ, false);
            this.a(ahb2, asv2, 4, 6, 2, 9, 6, 2, ajn.aJ, ajn.aJ, false);
            this.a(ahb2, asv2, 4, 6, 12, 8, 6, 12, ajn.aJ, ajn.aJ, false);
            this.a(ahb2, ajn.aJ, 0, 9, 6, 11, asv2);
            this.a(ahb2, ajn.aJ, 0, 8, 6, 11, asv2);
            this.a(ahb2, ajn.aJ, 0, 9, 6, 10, asv2);
            n2 = this.a(ajn.ap, 3);
            this.a(ahb2, ajn.ap, n2, 10, 1, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 2, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 3, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 4, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 5, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 6, 13, asv2);
            this.a(ahb2, ajn.ap, n2, 10, 7, 13, asv2);
            int n5 = 7;
            int n6 = 7;
            this.a(ahb2, ajn.aJ, 0, n5 - 1, 9, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5, 9, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 - 1, 8, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5, 8, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 - 1, 7, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5, 7, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 - 2, 7, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 + 1, 7, n6, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 - 1, 7, n6 - 1, asv2);
            this.a(ahb2, ajn.aJ, 0, n5 - 1, 7, n6 + 1, asv2);
            this.a(ahb2, ajn.aJ, 0, n5, 7, n6 - 1, asv2);
            this.a(ahb2, ajn.aJ, 0, n5, 7, n6 + 1, asv2);
            this.a(ahb2, ajn.aa, 0, n5 - 2, 8, n6, asv2);
            this.a(ahb2, ajn.aa, 0, n5 + 1, 8, n6, asv2);
            this.a(ahb2, ajn.aa, 0, n5 - 1, 8, n6 - 1, asv2);
            this.a(ahb2, ajn.aa, 0, n5 - 1, 8, n6 + 1, asv2);
            this.a(ahb2, ajn.aa, 0, n5, 8, n6 - 1, asv2);
            this.a(ahb2, ajn.aa, 0, n5, 8, n6 + 1, asv2);
        }
        this.a(ahb2, asv2, random, 3, 3, 5, qx.a(a, ade.bR.a(random, 1, 5, 2)), 1 + random.nextInt(4));
        if (this.b) {
            this.a(ahb2, ajn.a, 0, 12, 9, 1, asv2);
            this.a(ahb2, asv2, random, 12, 8, 1, qx.a(a, ade.bR.a(random, 1, 5, 2)), 1 + random.nextInt(4));
        }
        return true;
    }
}

