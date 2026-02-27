/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avq
extends awd {
    public avq() {
    }

    public avq(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static avq a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 9, 9, 6, n5);
        if (!avq.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avq(awa2, n6, random, asv2, n5);
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
            this.f.a(0, this.k - this.f.e + 9 - 1, 0);
        }
        this.a(ahb2, asv2, 1, 1, 1, 7, 5, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 0, 0, 8, 0, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 5, 0, 8, 5, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 6, 1, 8, 6, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 7, 2, 8, 7, 3, ajn.e, ajn.e, false);
        int n4 = this.a(ajn.ad, 3);
        int n5 = this.a(ajn.ad, 2);
        for (n3 = -1; n3 <= 2; ++n3) {
            for (n2 = 0; n2 <= 8; ++n2) {
                this.a(ahb2, ajn.ad, n4, n2, 6 + n3, n3, asv2);
                this.a(ahb2, ajn.ad, n5, n2, 6 + n3, 5 - n3, asv2);
            }
        }
        this.a(ahb2, asv2, 0, 1, 0, 0, 1, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 1, 5, 8, 1, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 8, 1, 0, 8, 1, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 2, 1, 0, 7, 1, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 2, 0, 0, 4, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 2, 5, 0, 4, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 8, 2, 5, 8, 4, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 8, 2, 0, 8, 4, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 2, 1, 0, 4, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 2, 5, 7, 4, 5, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 8, 2, 1, 8, 4, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 2, 0, 7, 4, 0, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.aZ, 0, 4, 2, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 2, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 6, 2, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 3, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 3, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 6, 3, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 3, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 3, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 3, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 3, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 5, asv2);
        this.a(ahb2, ajn.aZ, 0, 3, 2, 5, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 2, 5, asv2);
        this.a(ahb2, ajn.aZ, 0, 6, 2, 5, asv2);
        this.a(ahb2, asv2, 1, 4, 1, 7, 4, 1, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 4, 4, 7, 4, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 3, 4, 7, 3, 4, ajn.X, ajn.X, false);
        this.a(ahb2, ajn.f, 0, 7, 1, 4, asv2);
        this.a(ahb2, ajn.ad, this.a(ajn.ad, 0), 7, 1, 3, asv2);
        n3 = this.a(ajn.ad, 3);
        this.a(ahb2, ajn.ad, n3, 6, 1, 4, asv2);
        this.a(ahb2, ajn.ad, n3, 5, 1, 4, asv2);
        this.a(ahb2, ajn.ad, n3, 4, 1, 4, asv2);
        this.a(ahb2, ajn.ad, n3, 3, 1, 4, asv2);
        this.a(ahb2, ajn.aJ, 0, 6, 1, 3, asv2);
        this.a(ahb2, ajn.aw, 0, 6, 2, 3, asv2);
        this.a(ahb2, ajn.aJ, 0, 4, 1, 3, asv2);
        this.a(ahb2, ajn.aw, 0, 4, 2, 3, asv2);
        this.a(ahb2, ajn.ai, 0, 7, 1, 1, asv2);
        this.a(ahb2, ajn.a, 0, 1, 1, 0, asv2);
        this.a(ahb2, ajn.a, 0, 1, 2, 0, asv2);
        this.a(ahb2, asv2, random, 1, 1, 0, this.a(ajn.ao, 1));
        if (this.a(ahb2, 1, 0, -1, asv2).o() == awt.a && this.a(ahb2, 1, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 1, 0, -1, asv2);
        }
        for (n2 = 0; n2 < 6; ++n2) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.b(ahb2, i2, 9, n2, asv2);
                this.b(ahb2, ajn.e, 0, i2, -1, n2, asv2);
            }
        }
        this.a(ahb2, asv2, 2, 1, 2, 1);
        return true;
    }

    @Override
    protected int b(int n2) {
        return 1;
    }
}

