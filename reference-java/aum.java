/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aum
extends avc {
    private static final qx[] a = new qx[]{new qx(ade.bi, 0, 1, 1, 10), new qx(ade.i, 0, 1, 3, 3), new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 1, 3, 5), new qx(ade.ax, 0, 4, 9, 5), new qx(ade.P, 0, 1, 3, 15), new qx(ade.e, 0, 1, 3, 15), new qx(ade.b, 0, 1, 1, 5), new qx(ade.l, 0, 1, 1, 5), new qx(ade.Z, 0, 1, 1, 5), new qx(ade.Y, 0, 1, 1, 5), new qx(ade.aa, 0, 1, 1, 5), new qx(ade.ab, 0, 1, 1, 5), new qx(ade.ao, 0, 1, 1, 1), new qx(ade.av, 0, 1, 1, 1), new qx(ade.bX, 0, 1, 1, 1), new qx(ade.bY, 0, 1, 1, 1), new qx(ade.bZ, 0, 1, 1, 1)};
    private boolean b;

    public aum() {
    }

    public aum(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Chest", this.b);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.n("Chest");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((auz)avk2, list, random, 1, 1);
    }

    public static aum a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -1, 0, 5, 5, 7, n5);
        if (!aum.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new aum(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 4, 6, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 1, 0);
        this.a(ahb2, random, asv2, avd.a, 1, 1, 6);
        this.a(ahb2, asv2, 3, 1, 2, 3, 1, 4, ajn.aV, ajn.aV, false);
        this.a(ahb2, ajn.U, 5, 3, 1, 1, asv2);
        this.a(ahb2, ajn.U, 5, 3, 1, 5, asv2);
        this.a(ahb2, ajn.U, 5, 3, 2, 2, asv2);
        this.a(ahb2, ajn.U, 5, 3, 2, 4, asv2);
        for (n2 = 2; n2 <= 4; ++n2) {
            this.a(ahb2, ajn.U, 5, 2, 1, n2, asv2);
        }
        if (!this.b) {
            int n3;
            n2 = this.a(2);
            int n4 = this.a(3, 3);
            if (asv2.b(n4, n2, n3 = this.b(3, 3))) {
                this.b = true;
                this.a(ahb2, asv2, random, 3, 2, 3, qx.a(a, ade.bR.b(random)), 2 + random.nextInt(2));
            }
        }
        return true;
    }
}

