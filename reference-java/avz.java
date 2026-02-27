/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avz
extends awd {
    private static final qx[] a = new qx[]{new qx(ade.i, 0, 1, 3, 3), new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 1, 3, 5), new qx(ade.P, 0, 1, 3, 15), new qx(ade.e, 0, 1, 3, 15), new qx(ade.b, 0, 1, 1, 5), new qx(ade.l, 0, 1, 1, 5), new qx(ade.Z, 0, 1, 1, 5), new qx(ade.Y, 0, 1, 1, 5), new qx(ade.aa, 0, 1, 1, 5), new qx(ade.ab, 0, 1, 1, 5), new qx(adb.a(ajn.Z), 0, 3, 7, 5), new qx(adb.a(ajn.g), 0, 3, 7, 5), new qx(ade.av, 0, 1, 1, 3), new qx(ade.bX, 0, 1, 1, 1), new qx(ade.bY, 0, 1, 1, 1), new qx(ade.bZ, 0, 1, 1, 1)};
    private boolean b;

    public avz() {
    }

    public avz(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static avz a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 10, 6, 7, n5);
        if (!avz.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avz(awa2, n6, random, asv2, n5);
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
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 6 - 1, 0);
        }
        this.a(ahb2, asv2, 0, 1, 0, 9, 4, 6, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 0, 0, 9, 0, 6, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 4, 0, 9, 4, 6, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 5, 0, 9, 5, 6, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 1, 5, 1, 8, 5, 5, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 1, 0, 2, 3, 0, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 1, 0, 0, 4, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 3, 1, 0, 3, 4, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 0, 1, 6, 0, 4, 6, ajn.r, ajn.r, false);
        this.a(ahb2, ajn.f, 0, 3, 3, 1, asv2);
        this.a(ahb2, asv2, 3, 1, 2, 3, 3, 2, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 4, 1, 3, 5, 3, 3, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 1, 1, 0, 3, 5, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 1, 6, 5, 3, 6, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 5, 1, 0, 5, 3, 0, ajn.aJ, ajn.aJ, false);
        this.a(ahb2, asv2, 9, 1, 0, 9, 3, 0, ajn.aJ, ajn.aJ, false);
        this.a(ahb2, asv2, 6, 1, 4, 9, 4, 6, ajn.e, ajn.e, false);
        this.a(ahb2, ajn.k, 0, 7, 1, 5, asv2);
        this.a(ahb2, ajn.k, 0, 8, 1, 5, asv2);
        this.a(ahb2, ajn.aY, 0, 9, 2, 5, asv2);
        this.a(ahb2, ajn.aY, 0, 9, 2, 4, asv2);
        this.a(ahb2, asv2, 7, 2, 4, 8, 2, 5, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.e, 0, 6, 1, 3, asv2);
        this.a(ahb2, ajn.al, 0, 6, 2, 3, asv2);
        this.a(ahb2, ajn.al, 0, 6, 3, 3, asv2);
        this.a(ahb2, ajn.T, 0, 8, 1, 1, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 6, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 2, 6, asv2);
        this.a(ahb2, ajn.aJ, 0, 2, 1, 4, asv2);
        this.a(ahb2, ajn.aw, 0, 2, 2, 4, asv2);
        this.a(ahb2, ajn.f, 0, 1, 1, 5, asv2);
        this.a(ahb2, ajn.ad, this.a(ajn.ad, 3), 2, 1, 5, asv2);
        this.a(ahb2, ajn.ad, this.a(ajn.ad, 1), 1, 1, 4, asv2);
        if (!this.b) {
            int n4;
            n3 = this.a(1);
            n2 = this.a(5, 5);
            if (asv2.b(n2, n3, n4 = this.b(5, 5))) {
                this.b = true;
                this.a(ahb2, asv2, random, 5, 1, 5, a, 3 + random.nextInt(6));
            }
        }
        for (n3 = 6; n3 <= 8; ++n3) {
            if (this.a(ahb2, n3, 0, -1, asv2).o() != awt.a || this.a(ahb2, n3, -1, -1, asv2).o() == awt.a) continue;
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), n3, 0, -1, asv2);
        }
        for (n3 = 0; n3 < 7; ++n3) {
            for (n2 = 0; n2 < 10; ++n2) {
                this.b(ahb2, n2, 6, n3, asv2);
                this.b(ahb2, ajn.e, 0, n2, -1, n3, asv2);
            }
        }
        this.a(ahb2, asv2, 7, 1, 1, 1);
        return true;
    }

    @Override
    protected int b(int n2) {
        return 3;
    }
}

