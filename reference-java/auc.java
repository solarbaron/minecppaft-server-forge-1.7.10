/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class auc
extends aue {
    private boolean e;
    private boolean i;
    private boolean j;
    private boolean k;
    private static final qx[] l = new qx[]{new qx(ade.i, 0, 1, 3, 3), new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 2, 7, 15), new qx(ade.bC, 0, 1, 3, 2), new qx(ade.aS, 0, 4, 6, 20), new qx(ade.bh, 0, 3, 7, 16), new qx(ade.av, 0, 1, 1, 3), new qx(ade.bX, 0, 1, 1, 1), new qx(ade.bY, 0, 1, 1, 1), new qx(ade.bZ, 0, 1, 1, 1)};
    private static final qx[] m = new qx[]{new qx(ade.g, 0, 2, 7, 30)};
    private static aud n = new aud(null);

    public auc() {
    }

    public auc(Random random, int n2, int n3) {
        super(random, n2, 64, n3, 12, 10, 15);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("placedMainChest", this.e);
        dh2.a("placedHiddenChest", this.i);
        dh2.a("placedTrap1", this.j);
        dh2.a("placedTrap2", this.k);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.e = dh2.n("placedMainChest");
        this.i = dh2.n("placedHiddenChest");
        this.j = dh2.n("placedTrap1");
        this.k = dh2.n("placedTrap2");
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (!this.a(ahb2, asv2, 0)) {
            return false;
        }
        int n3 = this.a(ajn.ar, 3);
        int n4 = this.a(ajn.ar, 2);
        int n5 = this.a(ajn.ar, 0);
        int n6 = this.a(ajn.ar, 1);
        this.a(ahb2, asv2, 0, -4, 0, this.a - 1, 0, this.c - 1, false, random, n);
        this.a(ahb2, asv2, 2, 1, 2, 9, 2, 2, false, random, n);
        this.a(ahb2, asv2, 2, 1, 12, 9, 2, 12, false, random, n);
        this.a(ahb2, asv2, 2, 1, 3, 2, 2, 11, false, random, n);
        this.a(ahb2, asv2, 9, 1, 3, 9, 2, 11, false, random, n);
        this.a(ahb2, asv2, 1, 3, 1, 10, 6, 1, false, random, n);
        this.a(ahb2, asv2, 1, 3, 13, 10, 6, 13, false, random, n);
        this.a(ahb2, asv2, 1, 3, 2, 1, 6, 12, false, random, n);
        this.a(ahb2, asv2, 10, 3, 2, 10, 6, 12, false, random, n);
        this.a(ahb2, asv2, 2, 3, 2, 9, 3, 12, false, random, n);
        this.a(ahb2, asv2, 2, 6, 2, 9, 6, 12, false, random, n);
        this.a(ahb2, asv2, 3, 7, 3, 8, 7, 11, false, random, n);
        this.a(ahb2, asv2, 4, 8, 4, 7, 8, 10, false, random, n);
        this.a(ahb2, asv2, 3, 1, 3, 8, 2, 11);
        this.a(ahb2, asv2, 4, 3, 6, 7, 3, 9);
        this.a(ahb2, asv2, 2, 4, 2, 9, 5, 12);
        this.a(ahb2, asv2, 4, 6, 5, 7, 6, 9);
        this.a(ahb2, asv2, 5, 7, 6, 6, 7, 8);
        this.a(ahb2, asv2, 5, 1, 2, 6, 2, 2);
        this.a(ahb2, asv2, 5, 2, 12, 6, 2, 12);
        this.a(ahb2, asv2, 5, 5, 1, 6, 5, 1);
        this.a(ahb2, asv2, 5, 5, 13, 6, 5, 13);
        this.a(ahb2, ajn.a, 0, 1, 5, 5, asv2);
        this.a(ahb2, ajn.a, 0, 10, 5, 5, asv2);
        this.a(ahb2, ajn.a, 0, 1, 5, 9, asv2);
        this.a(ahb2, ajn.a, 0, 10, 5, 9, asv2);
        for (n2 = 0; n2 <= 14; n2 += 14) {
            this.a(ahb2, asv2, 2, 4, n2, 2, 5, n2, false, random, n);
            this.a(ahb2, asv2, 4, 4, n2, 4, 5, n2, false, random, n);
            this.a(ahb2, asv2, 7, 4, n2, 7, 5, n2, false, random, n);
            this.a(ahb2, asv2, 9, 4, n2, 9, 5, n2, false, random, n);
        }
        this.a(ahb2, asv2, 5, 6, 0, 6, 6, 0, false, random, n);
        for (n2 = 0; n2 <= 11; n2 += 11) {
            for (int i2 = 2; i2 <= 12; i2 += 2) {
                this.a(ahb2, asv2, n2, 4, i2, n2, 5, i2, false, random, n);
            }
            this.a(ahb2, asv2, n2, 6, 5, n2, 6, 5, false, random, n);
            this.a(ahb2, asv2, n2, 6, 9, n2, 6, 9, false, random, n);
        }
        this.a(ahb2, asv2, 2, 7, 2, 2, 9, 2, false, random, n);
        this.a(ahb2, asv2, 9, 7, 2, 9, 9, 2, false, random, n);
        this.a(ahb2, asv2, 2, 7, 12, 2, 9, 12, false, random, n);
        this.a(ahb2, asv2, 9, 7, 12, 9, 9, 12, false, random, n);
        this.a(ahb2, asv2, 4, 9, 4, 4, 9, 4, false, random, n);
        this.a(ahb2, asv2, 7, 9, 4, 7, 9, 4, false, random, n);
        this.a(ahb2, asv2, 4, 9, 10, 4, 9, 10, false, random, n);
        this.a(ahb2, asv2, 7, 9, 10, 7, 9, 10, false, random, n);
        this.a(ahb2, asv2, 5, 9, 7, 6, 9, 7, false, random, n);
        this.a(ahb2, ajn.ar, n3, 5, 9, 6, asv2);
        this.a(ahb2, ajn.ar, n3, 6, 9, 6, asv2);
        this.a(ahb2, ajn.ar, n4, 5, 9, 8, asv2);
        this.a(ahb2, ajn.ar, n4, 6, 9, 8, asv2);
        this.a(ahb2, ajn.ar, n3, 4, 0, 0, asv2);
        this.a(ahb2, ajn.ar, n3, 5, 0, 0, asv2);
        this.a(ahb2, ajn.ar, n3, 6, 0, 0, asv2);
        this.a(ahb2, ajn.ar, n3, 7, 0, 0, asv2);
        this.a(ahb2, ajn.ar, n3, 4, 1, 8, asv2);
        this.a(ahb2, ajn.ar, n3, 4, 2, 9, asv2);
        this.a(ahb2, ajn.ar, n3, 4, 3, 10, asv2);
        this.a(ahb2, ajn.ar, n3, 7, 1, 8, asv2);
        this.a(ahb2, ajn.ar, n3, 7, 2, 9, asv2);
        this.a(ahb2, ajn.ar, n3, 7, 3, 10, asv2);
        this.a(ahb2, asv2, 4, 1, 9, 4, 1, 9, false, random, n);
        this.a(ahb2, asv2, 7, 1, 9, 7, 1, 9, false, random, n);
        this.a(ahb2, asv2, 4, 1, 10, 7, 2, 10, false, random, n);
        this.a(ahb2, asv2, 5, 4, 5, 6, 4, 5, false, random, n);
        this.a(ahb2, ajn.ar, n5, 4, 4, 5, asv2);
        this.a(ahb2, ajn.ar, n6, 7, 4, 5, asv2);
        for (n2 = 0; n2 < 4; ++n2) {
            this.a(ahb2, ajn.ar, n4, 5, 0 - n2, 6 + n2, asv2);
            this.a(ahb2, ajn.ar, n4, 6, 0 - n2, 6 + n2, asv2);
            this.a(ahb2, asv2, 5, 0 - n2, 7 + n2, 6, 0 - n2, 9 + n2);
        }
        this.a(ahb2, asv2, 1, -3, 12, 10, -1, 13);
        this.a(ahb2, asv2, 1, -3, 1, 3, -1, 13);
        this.a(ahb2, asv2, 1, -3, 1, 9, -1, 5);
        for (n2 = 1; n2 <= 13; n2 += 2) {
            this.a(ahb2, asv2, 1, -3, n2, 1, -2, n2, false, random, n);
        }
        for (n2 = 2; n2 <= 12; n2 += 2) {
            this.a(ahb2, asv2, 1, -1, n2, 3, -1, n2, false, random, n);
        }
        this.a(ahb2, asv2, 2, -2, 1, 5, -2, 1, false, random, n);
        this.a(ahb2, asv2, 7, -2, 1, 9, -2, 1, false, random, n);
        this.a(ahb2, asv2, 6, -3, 1, 6, -3, 1, false, random, n);
        this.a(ahb2, asv2, 6, -1, 1, 6, -1, 1, false, random, n);
        this.a(ahb2, ajn.bC, this.a(ajn.bC, 3) | 4, 1, -3, 8, asv2);
        this.a(ahb2, ajn.bC, this.a(ajn.bC, 1) | 4, 4, -3, 8, asv2);
        this.a(ahb2, ajn.bD, 4, 2, -3, 8, asv2);
        this.a(ahb2, ajn.bD, 4, 3, -3, 8, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 7, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 6, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 5, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 4, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 3, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 2, asv2);
        this.a(ahb2, ajn.af, 0, 5, -3, 1, asv2);
        this.a(ahb2, ajn.af, 0, 4, -3, 1, asv2);
        this.a(ahb2, ajn.Y, 0, 3, -3, 1, asv2);
        if (!this.j) {
            this.j = this.a(ahb2, asv2, random, 3, -2, 1, 2, m, 2);
        }
        this.a(ahb2, ajn.bd, 15, 3, -2, 2, asv2);
        this.a(ahb2, ajn.bC, this.a(ajn.bC, 2) | 4, 7, -3, 1, asv2);
        this.a(ahb2, ajn.bC, this.a(ajn.bC, 0) | 4, 7, -3, 5, asv2);
        this.a(ahb2, ajn.bD, 4, 7, -3, 2, asv2);
        this.a(ahb2, ajn.bD, 4, 7, -3, 3, asv2);
        this.a(ahb2, ajn.bD, 4, 7, -3, 4, asv2);
        this.a(ahb2, ajn.af, 0, 8, -3, 6, asv2);
        this.a(ahb2, ajn.af, 0, 9, -3, 6, asv2);
        this.a(ahb2, ajn.af, 0, 9, -3, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 9, -3, 4, asv2);
        this.a(ahb2, ajn.af, 0, 9, -2, 4, asv2);
        if (!this.k) {
            this.k = this.a(ahb2, asv2, random, 9, -2, 3, 4, m, 2);
        }
        this.a(ahb2, ajn.bd, 15, 8, -1, 3, asv2);
        this.a(ahb2, ajn.bd, 15, 8, -2, 3, asv2);
        if (!this.e) {
            this.e = this.a(ahb2, asv2, random, 8, -3, 3, qx.a(l, ade.bR.b(random)), 2 + random.nextInt(5));
        }
        this.a(ahb2, ajn.Y, 0, 9, -3, 2, asv2);
        this.a(ahb2, ajn.Y, 0, 8, -3, 1, asv2);
        this.a(ahb2, ajn.Y, 0, 4, -3, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 5, -2, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 5, -1, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 6, -3, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 7, -2, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 7, -1, 5, asv2);
        this.a(ahb2, ajn.Y, 0, 8, -3, 5, asv2);
        this.a(ahb2, asv2, 9, -1, 1, 9, -1, 5, false, random, n);
        this.a(ahb2, asv2, 8, -3, 8, 10, -1, 10);
        this.a(ahb2, ajn.aV, 3, 8, -2, 11, asv2);
        this.a(ahb2, ajn.aV, 3, 9, -2, 11, asv2);
        this.a(ahb2, ajn.aV, 3, 10, -2, 11, asv2);
        this.a(ahb2, ajn.at, alv.b(this.a(ajn.at, 2)), 8, -2, 12, asv2);
        this.a(ahb2, ajn.at, alv.b(this.a(ajn.at, 2)), 9, -2, 12, asv2);
        this.a(ahb2, ajn.at, alv.b(this.a(ajn.at, 2)), 10, -2, 12, asv2);
        this.a(ahb2, asv2, 8, -3, 8, 8, -3, 10, false, random, n);
        this.a(ahb2, asv2, 10, -3, 8, 10, -3, 10, false, random, n);
        this.a(ahb2, ajn.Y, 0, 10, -2, 9, asv2);
        this.a(ahb2, ajn.af, 0, 8, -2, 9, asv2);
        this.a(ahb2, ajn.af, 0, 8, -2, 10, asv2);
        this.a(ahb2, ajn.af, 0, 10, -1, 9, asv2);
        this.a(ahb2, ajn.F, 1, 9, -2, 8, asv2);
        this.a(ahb2, ajn.F, this.a(ajn.F, 4), 10, -2, 8, asv2);
        this.a(ahb2, ajn.F, this.a(ajn.F, 4), 10, -1, 8, asv2);
        this.a(ahb2, ajn.aR, this.a(ajn.aR, 2), 10, -2, 10, asv2);
        if (!this.i) {
            this.i = this.a(ahb2, asv2, random, 9, -3, 10, qx.a(l, ade.bR.b(random)), 2 + random.nextInt(5));
        }
        return true;
    }
}

