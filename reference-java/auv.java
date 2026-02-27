/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auv
extends avc {
    private static final qx[] b = new qx[]{new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 1, 3, 5), new qx(ade.ax, 0, 4, 9, 5), new qx(ade.h, 0, 3, 8, 10), new qx(ade.P, 0, 1, 3, 15), new qx(ade.e, 0, 1, 3, 15), new qx(ade.b, 0, 1, 1, 1)};
    protected int a;

    public auv() {
    }

    public auv(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
        this.a = random.nextInt(5);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Type", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.f("Type");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        this.a((auz)avk2, list, random, 4, 1);
        this.b((auz)avk2, list, random, 1, 4);
        this.c((auz)avk2, list, random, 1, 4);
    }

    public static auv a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -4, -1, 0, 11, 7, 11, n5);
        if (!auv.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new auv(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 10, 6, 10, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 4, 1, 0);
        this.a(ahb2, asv2, 4, 1, 10, 6, 3, 10, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 1, 4, 0, 3, 6, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 10, 1, 4, 10, 3, 6, ajn.a, ajn.a, false);
        switch (this.a) {
            default: {
                break;
            }
            case 0: {
                this.a(ahb2, ajn.aV, 0, 5, 1, 5, asv2);
                this.a(ahb2, ajn.aV, 0, 5, 2, 5, asv2);
                this.a(ahb2, ajn.aV, 0, 5, 3, 5, asv2);
                this.a(ahb2, ajn.aa, 0, 4, 3, 5, asv2);
                this.a(ahb2, ajn.aa, 0, 6, 3, 5, asv2);
                this.a(ahb2, ajn.aa, 0, 5, 3, 4, asv2);
                this.a(ahb2, ajn.aa, 0, 5, 3, 6, asv2);
                this.a(ahb2, ajn.U, 0, 4, 1, 4, asv2);
                this.a(ahb2, ajn.U, 0, 4, 1, 5, asv2);
                this.a(ahb2, ajn.U, 0, 4, 1, 6, asv2);
                this.a(ahb2, ajn.U, 0, 6, 1, 4, asv2);
                this.a(ahb2, ajn.U, 0, 6, 1, 5, asv2);
                this.a(ahb2, ajn.U, 0, 6, 1, 6, asv2);
                this.a(ahb2, ajn.U, 0, 5, 1, 4, asv2);
                this.a(ahb2, ajn.U, 0, 5, 1, 6, asv2);
                break;
            }
            case 1: {
                for (int i2 = 0; i2 < 5; ++i2) {
                    this.a(ahb2, ajn.aV, 0, 3, 1, 3 + i2, asv2);
                    this.a(ahb2, ajn.aV, 0, 7, 1, 3 + i2, asv2);
                    this.a(ahb2, ajn.aV, 0, 3 + i2, 1, 3, asv2);
                    this.a(ahb2, ajn.aV, 0, 3 + i2, 1, 7, asv2);
                }
                this.a(ahb2, ajn.aV, 0, 5, 1, 5, asv2);
                this.a(ahb2, ajn.aV, 0, 5, 2, 5, asv2);
                this.a(ahb2, ajn.aV, 0, 5, 3, 5, asv2);
                this.a(ahb2, ajn.i, 0, 5, 4, 5, asv2);
                break;
            }
            case 2: {
                int n2;
                for (n2 = 1; n2 <= 9; ++n2) {
                    this.a(ahb2, ajn.e, 0, 1, 3, n2, asv2);
                    this.a(ahb2, ajn.e, 0, 9, 3, n2, asv2);
                }
                for (n2 = 1; n2 <= 9; ++n2) {
                    this.a(ahb2, ajn.e, 0, n2, 3, 1, asv2);
                    this.a(ahb2, ajn.e, 0, n2, 3, 9, asv2);
                }
                this.a(ahb2, ajn.e, 0, 5, 1, 4, asv2);
                this.a(ahb2, ajn.e, 0, 5, 1, 6, asv2);
                this.a(ahb2, ajn.e, 0, 5, 3, 4, asv2);
                this.a(ahb2, ajn.e, 0, 5, 3, 6, asv2);
                this.a(ahb2, ajn.e, 0, 4, 1, 5, asv2);
                this.a(ahb2, ajn.e, 0, 6, 1, 5, asv2);
                this.a(ahb2, ajn.e, 0, 4, 3, 5, asv2);
                this.a(ahb2, ajn.e, 0, 6, 3, 5, asv2);
                for (n2 = 1; n2 <= 3; ++n2) {
                    this.a(ahb2, ajn.e, 0, 4, n2, 4, asv2);
                    this.a(ahb2, ajn.e, 0, 6, n2, 4, asv2);
                    this.a(ahb2, ajn.e, 0, 4, n2, 6, asv2);
                    this.a(ahb2, ajn.e, 0, 6, n2, 6, asv2);
                }
                this.a(ahb2, ajn.aa, 0, 5, 3, 5, asv2);
                for (n2 = 2; n2 <= 8; ++n2) {
                    this.a(ahb2, ajn.f, 0, 2, 3, n2, asv2);
                    this.a(ahb2, ajn.f, 0, 3, 3, n2, asv2);
                    if (n2 <= 3 || n2 >= 7) {
                        this.a(ahb2, ajn.f, 0, 4, 3, n2, asv2);
                        this.a(ahb2, ajn.f, 0, 5, 3, n2, asv2);
                        this.a(ahb2, ajn.f, 0, 6, 3, n2, asv2);
                    }
                    this.a(ahb2, ajn.f, 0, 7, 3, n2, asv2);
                    this.a(ahb2, ajn.f, 0, 8, 3, n2, asv2);
                }
                this.a(ahb2, ajn.ap, this.a(ajn.ap, 4), 9, 1, 3, asv2);
                this.a(ahb2, ajn.ap, this.a(ajn.ap, 4), 9, 2, 3, asv2);
                this.a(ahb2, ajn.ap, this.a(ajn.ap, 4), 9, 3, 3, asv2);
                this.a(ahb2, asv2, random, 3, 4, 8, qx.a(b, ade.bR.b(random)), 1 + random.nextInt(4));
            }
        }
        return true;
    }
}

