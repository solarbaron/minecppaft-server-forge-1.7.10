/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aub
extends aue {
    private boolean[] e = new boolean[4];
    private static final qx[] i = new qx[]{new qx(ade.i, 0, 1, 3, 3), new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 2, 7, 15), new qx(ade.bC, 0, 1, 3, 2), new qx(ade.aS, 0, 4, 6, 20), new qx(ade.bh, 0, 3, 7, 16), new qx(ade.av, 0, 1, 1, 3), new qx(ade.bX, 0, 1, 1, 1), new qx(ade.bY, 0, 1, 1, 1), new qx(ade.bZ, 0, 1, 1, 1)};

    public aub() {
    }

    public aub(Random random, int n2, int n3) {
        super(random, n2, 64, n3, 21, 15, 21);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("hasPlacedChest0", this.e[0]);
        dh2.a("hasPlacedChest1", this.e[1]);
        dh2.a("hasPlacedChest2", this.e[2]);
        dh2.a("hasPlacedChest3", this.e[3]);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.e[0] = dh2.n("hasPlacedChest0");
        this.e[1] = dh2.n("hasPlacedChest1");
        this.e[2] = dh2.n("hasPlacedChest2");
        this.e[3] = dh2.n("hasPlacedChest3");
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        int n4;
        int n5;
        this.a(ahb2, asv2, 0, -4, 0, this.a - 1, 0, this.c - 1, ajn.A, ajn.A, false);
        for (n5 = 1; n5 <= 9; ++n5) {
            this.a(ahb2, asv2, n5, n5, n5, this.a - 1 - n5, n5, this.c - 1 - n5, ajn.A, ajn.A, false);
            this.a(ahb2, asv2, n5 + 1, n5, n5 + 1, this.a - 2 - n5, n5, this.c - 2 - n5, ajn.a, ajn.a, false);
        }
        for (n5 = 0; n5 < this.a; ++n5) {
            for (n4 = 0; n4 < this.c; ++n4) {
                n3 = -5;
                this.b(ahb2, ajn.A, 0, n5, n3, n4, asv2);
            }
        }
        n5 = this.a(ajn.bz, 3);
        n4 = this.a(ajn.bz, 2);
        n3 = this.a(ajn.bz, 0);
        int n6 = this.a(ajn.bz, 1);
        int n7 = 1;
        int n8 = 11;
        this.a(ahb2, asv2, 0, 0, 0, 4, 9, 4, ajn.A, ajn.a, false);
        this.a(ahb2, asv2, 1, 10, 1, 3, 10, 3, ajn.A, ajn.A, false);
        this.a(ahb2, ajn.bz, n5, 2, 10, 0, asv2);
        this.a(ahb2, ajn.bz, n4, 2, 10, 4, asv2);
        this.a(ahb2, ajn.bz, n3, 0, 10, 2, asv2);
        this.a(ahb2, ajn.bz, n6, 4, 10, 2, asv2);
        this.a(ahb2, asv2, this.a - 5, 0, 0, this.a - 1, 9, 4, ajn.A, ajn.a, false);
        this.a(ahb2, asv2, this.a - 4, 10, 1, this.a - 2, 10, 3, ajn.A, ajn.A, false);
        this.a(ahb2, ajn.bz, n5, this.a - 3, 10, 0, asv2);
        this.a(ahb2, ajn.bz, n4, this.a - 3, 10, 4, asv2);
        this.a(ahb2, ajn.bz, n3, this.a - 5, 10, 2, asv2);
        this.a(ahb2, ajn.bz, n6, this.a - 1, 10, 2, asv2);
        this.a(ahb2, asv2, 8, 0, 0, 12, 4, 4, ajn.A, ajn.a, false);
        this.a(ahb2, asv2, 9, 1, 0, 11, 3, 4, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.A, 2, 9, 1, 1, asv2);
        this.a(ahb2, ajn.A, 2, 9, 2, 1, asv2);
        this.a(ahb2, ajn.A, 2, 9, 3, 1, asv2);
        this.a(ahb2, ajn.A, 2, 10, 3, 1, asv2);
        this.a(ahb2, ajn.A, 2, 11, 3, 1, asv2);
        this.a(ahb2, ajn.A, 2, 11, 2, 1, asv2);
        this.a(ahb2, ajn.A, 2, 11, 1, 1, asv2);
        this.a(ahb2, asv2, 4, 1, 1, 8, 3, 3, ajn.A, ajn.a, false);
        this.a(ahb2, asv2, 4, 1, 2, 8, 2, 2, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 12, 1, 1, 16, 3, 3, ajn.A, ajn.a, false);
        this.a(ahb2, asv2, 12, 1, 2, 16, 2, 2, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 5, 4, 5, this.a - 6, 4, this.c - 6, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, 9, 4, 9, 11, 4, 11, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 8, 1, 8, 8, 3, 8, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 12, 1, 8, 12, 3, 8, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 8, 1, 12, 8, 3, 12, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 12, 1, 12, 12, 3, 12, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 1, 1, 5, 4, 4, 11, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, this.a - 5, 1, 5, this.a - 2, 4, 11, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, 6, 7, 9, 6, 7, 11, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, this.a - 7, 7, 9, this.a - 7, 7, 11, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, 5, 5, 9, 5, 7, 11, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, this.a - 6, 5, 9, this.a - 6, 7, 11, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, ajn.a, 0, 5, 5, 10, asv2);
        this.a(ahb2, ajn.a, 0, 5, 6, 10, asv2);
        this.a(ahb2, ajn.a, 0, 6, 6, 10, asv2);
        this.a(ahb2, ajn.a, 0, this.a - 6, 5, 10, asv2);
        this.a(ahb2, ajn.a, 0, this.a - 6, 6, 10, asv2);
        this.a(ahb2, ajn.a, 0, this.a - 7, 6, 10, asv2);
        this.a(ahb2, asv2, 2, 4, 4, 2, 6, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, this.a - 3, 4, 4, this.a - 3, 6, 4, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.bz, n5, 2, 4, 5, asv2);
        this.a(ahb2, ajn.bz, n5, 2, 3, 4, asv2);
        this.a(ahb2, ajn.bz, n5, this.a - 3, 4, 5, asv2);
        this.a(ahb2, ajn.bz, n5, this.a - 3, 3, 4, asv2);
        this.a(ahb2, asv2, 1, 1, 3, 2, 2, 3, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, this.a - 3, 1, 3, this.a - 2, 2, 3, ajn.A, ajn.A, false);
        this.a(ahb2, ajn.bz, 0, 1, 1, 2, asv2);
        this.a(ahb2, ajn.bz, 0, this.a - 2, 1, 2, asv2);
        this.a(ahb2, ajn.U, 1, 1, 2, 2, asv2);
        this.a(ahb2, ajn.U, 1, this.a - 2, 2, 2, asv2);
        this.a(ahb2, ajn.bz, n6, 2, 1, 2, asv2);
        this.a(ahb2, ajn.bz, n3, this.a - 3, 1, 2, asv2);
        this.a(ahb2, asv2, 4, 3, 5, 4, 3, 18, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, this.a - 5, 3, 5, this.a - 5, 3, 17, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, 3, 1, 5, 4, 2, 16, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, this.a - 6, 1, 5, this.a - 5, 2, 16, ajn.a, ajn.a, false);
        for (n2 = 5; n2 <= 17; n2 += 2) {
            this.a(ahb2, ajn.A, 2, 4, 1, n2, asv2);
            this.a(ahb2, ajn.A, 1, 4, 2, n2, asv2);
            this.a(ahb2, ajn.A, 2, this.a - 5, 1, n2, asv2);
            this.a(ahb2, ajn.A, 1, this.a - 5, 2, n2, asv2);
        }
        this.a(ahb2, ajn.L, n7, 10, 0, 7, asv2);
        this.a(ahb2, ajn.L, n7, 10, 0, 8, asv2);
        this.a(ahb2, ajn.L, n7, 9, 0, 9, asv2);
        this.a(ahb2, ajn.L, n7, 11, 0, 9, asv2);
        this.a(ahb2, ajn.L, n7, 8, 0, 10, asv2);
        this.a(ahb2, ajn.L, n7, 12, 0, 10, asv2);
        this.a(ahb2, ajn.L, n7, 7, 0, 10, asv2);
        this.a(ahb2, ajn.L, n7, 13, 0, 10, asv2);
        this.a(ahb2, ajn.L, n7, 9, 0, 11, asv2);
        this.a(ahb2, ajn.L, n7, 11, 0, 11, asv2);
        this.a(ahb2, ajn.L, n7, 10, 0, 12, asv2);
        this.a(ahb2, ajn.L, n7, 10, 0, 13, asv2);
        this.a(ahb2, ajn.L, n8, 10, 0, 10, asv2);
        for (n2 = 0; n2 <= this.a - 1; n2 += this.a - 1) {
            this.a(ahb2, ajn.A, 2, n2, 2, 1, asv2);
            this.a(ahb2, ajn.L, n7, n2, 2, 2, asv2);
            this.a(ahb2, ajn.A, 2, n2, 2, 3, asv2);
            this.a(ahb2, ajn.A, 2, n2, 3, 1, asv2);
            this.a(ahb2, ajn.L, n7, n2, 3, 2, asv2);
            this.a(ahb2, ajn.A, 2, n2, 3, 3, asv2);
            this.a(ahb2, ajn.L, n7, n2, 4, 1, asv2);
            this.a(ahb2, ajn.A, 1, n2, 4, 2, asv2);
            this.a(ahb2, ajn.L, n7, n2, 4, 3, asv2);
            this.a(ahb2, ajn.A, 2, n2, 5, 1, asv2);
            this.a(ahb2, ajn.L, n7, n2, 5, 2, asv2);
            this.a(ahb2, ajn.A, 2, n2, 5, 3, asv2);
            this.a(ahb2, ajn.L, n7, n2, 6, 1, asv2);
            this.a(ahb2, ajn.A, 1, n2, 6, 2, asv2);
            this.a(ahb2, ajn.L, n7, n2, 6, 3, asv2);
            this.a(ahb2, ajn.L, n7, n2, 7, 1, asv2);
            this.a(ahb2, ajn.L, n7, n2, 7, 2, asv2);
            this.a(ahb2, ajn.L, n7, n2, 7, 3, asv2);
            this.a(ahb2, ajn.A, 2, n2, 8, 1, asv2);
            this.a(ahb2, ajn.A, 2, n2, 8, 2, asv2);
            this.a(ahb2, ajn.A, 2, n2, 8, 3, asv2);
        }
        for (n2 = 2; n2 <= this.a - 3; n2 += this.a - 3 - 2) {
            this.a(ahb2, ajn.A, 2, n2 - 1, 2, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2, 2, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 + 1, 2, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 - 1, 3, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2, 3, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 + 1, 3, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 - 1, 4, 0, asv2);
            this.a(ahb2, ajn.A, 1, n2, 4, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 + 1, 4, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 - 1, 5, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2, 5, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 + 1, 5, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 - 1, 6, 0, asv2);
            this.a(ahb2, ajn.A, 1, n2, 6, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 + 1, 6, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 - 1, 7, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2, 7, 0, asv2);
            this.a(ahb2, ajn.L, n7, n2 + 1, 7, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 - 1, 8, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2, 8, 0, asv2);
            this.a(ahb2, ajn.A, 2, n2 + 1, 8, 0, asv2);
        }
        this.a(ahb2, asv2, 8, 4, 0, 12, 6, 0, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, ajn.a, 0, 8, 6, 0, asv2);
        this.a(ahb2, ajn.a, 0, 12, 6, 0, asv2);
        this.a(ahb2, ajn.L, n7, 9, 5, 0, asv2);
        this.a(ahb2, ajn.A, 1, 10, 5, 0, asv2);
        this.a(ahb2, ajn.L, n7, 11, 5, 0, asv2);
        this.a(ahb2, asv2, 8, -14, 8, 12, -11, 12, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 8, -10, 8, 12, -10, 12, ajn.A, 1, ajn.A, 1, false);
        this.a(ahb2, asv2, 8, -9, 8, 12, -9, 12, ajn.A, 2, ajn.A, 2, false);
        this.a(ahb2, asv2, 8, -8, 8, 12, -1, 12, ajn.A, ajn.A, false);
        this.a(ahb2, asv2, 9, -11, 9, 11, -1, 11, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.au, 0, 10, -11, 10, asv2);
        this.a(ahb2, asv2, 9, -13, 9, 11, -13, 11, ajn.W, ajn.a, false);
        this.a(ahb2, ajn.a, 0, 8, -11, 10, asv2);
        this.a(ahb2, ajn.a, 0, 8, -10, 10, asv2);
        this.a(ahb2, ajn.A, 1, 7, -10, 10, asv2);
        this.a(ahb2, ajn.A, 2, 7, -11, 10, asv2);
        this.a(ahb2, ajn.a, 0, 12, -11, 10, asv2);
        this.a(ahb2, ajn.a, 0, 12, -10, 10, asv2);
        this.a(ahb2, ajn.A, 1, 13, -10, 10, asv2);
        this.a(ahb2, ajn.A, 2, 13, -11, 10, asv2);
        this.a(ahb2, ajn.a, 0, 10, -11, 8, asv2);
        this.a(ahb2, ajn.a, 0, 10, -10, 8, asv2);
        this.a(ahb2, ajn.A, 1, 10, -10, 7, asv2);
        this.a(ahb2, ajn.A, 2, 10, -11, 7, asv2);
        this.a(ahb2, ajn.a, 0, 10, -11, 12, asv2);
        this.a(ahb2, ajn.a, 0, 10, -10, 12, asv2);
        this.a(ahb2, ajn.A, 1, 10, -10, 13, asv2);
        this.a(ahb2, ajn.A, 2, 10, -11, 13, asv2);
        for (n2 = 0; n2 < 4; ++n2) {
            if (this.e[n2]) continue;
            int n9 = p.a[n2] * 2;
            int n10 = p.b[n2] * 2;
            this.e[n2] = this.a(ahb2, asv2, random, 10 + n9, -11, 10 + n10, qx.a(i, ade.bR.b(random)), 2 + random.nextInt(5));
        }
        return true;
    }
}

