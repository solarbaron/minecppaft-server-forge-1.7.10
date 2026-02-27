/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avv
extends awd {
    public avv() {
    }

    public avv(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static avv a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 9, 7, 11, n5);
        if (!avv.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avv(awa2, n6, random, asv2, n5);
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
            this.f.a(0, this.k - this.f.e + 7 - 1, 0);
        }
        this.a(ahb2, asv2, 1, 1, 1, 7, 4, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 2, 1, 6, 8, 4, 10, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 2, 0, 6, 8, 0, 10, ajn.d, ajn.d, false);
        this.a(ahb2, ajn.e, 0, 6, 0, 6, asv2);
        this.a(ahb2, asv2, 2, 1, 6, 2, 1, 10, ajn.aJ, ajn.aJ, false);
        this.a(ahb2, asv2, 8, 1, 6, 8, 1, 10, ajn.aJ, ajn.aJ, false);
        this.a(ahb2, asv2, 3, 1, 10, 7, 1, 10, ajn.aJ, ajn.aJ, false);
        this.a(ahb2, asv2, 1, 0, 1, 7, 0, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 0, 0, 0, 3, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 8, 0, 0, 8, 3, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 0, 0, 7, 1, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 0, 5, 7, 1, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 2, 0, 7, 3, 0, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 2, 5, 7, 3, 5, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 4, 1, 8, 4, 1, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 4, 4, 8, 4, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 5, 2, 8, 5, 3, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.f, 0, 0, 4, 2, asv2);
        this.a(ahb2, ajn.f, 0, 0, 4, 3, asv2);
        this.a(ahb2, ajn.f, 0, 8, 4, 2, asv2);
        this.a(ahb2, ajn.f, 0, 8, 4, 3, asv2);
        int n4 = this.a(ajn.ad, 3);
        int n5 = this.a(ajn.ad, 2);
        for (n3 = -1; n3 <= 2; ++n3) {
            for (n2 = 0; n2 <= 8; ++n2) {
                this.a(ahb2, ajn.ad, n4, n2, 4 + n3, n3, asv2);
                this.a(ahb2, ajn.ad, n5, n2, 4 + n3, 5 - n3, asv2);
            }
        }
        this.a(ahb2, ajn.r, 0, 0, 2, 1, asv2);
        this.a(ahb2, ajn.r, 0, 0, 2, 4, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 1, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 3, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 5, asv2);
        this.a(ahb2, ajn.aZ, 0, 3, 2, 5, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 2, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 6, 2, 5, asv2);
        this.a(ahb2, ajn.aJ, 0, 2, 1, 3, asv2);
        this.a(ahb2, ajn.aw, 0, 2, 2, 3, asv2);
        this.a(ahb2, ajn.f, 0, 1, 1, 4, asv2);
        this.a(ahb2, ajn.ad, this.a(ajn.ad, 3), 2, 1, 4, asv2);
        this.a(ahb2, ajn.ad, this.a(ajn.ad, 1), 1, 1, 3, asv2);
        this.a(ahb2, asv2, 5, 0, 1, 7, 0, 3, ajn.T, ajn.T, false);
        this.a(ahb2, ajn.T, 0, 6, 1, 1, asv2);
        this.a(ahb2, ajn.T, 0, 6, 1, 2, asv2);
        this.a(ahb2, ajn.a, 0, 2, 1, 0, asv2);
        this.a(ahb2, ajn.a, 0, 2, 2, 0, asv2);
        this.a(ahb2, ajn.aa, 0, 2, 3, 1, asv2);
        this.a(ahb2, asv2, random, 2, 1, 0, this.a(ajn.ao, 1));
        if (this.a(ahb2, 2, 0, -1, asv2).o() == awt.a && this.a(ahb2, 2, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 2, 0, -1, asv2);
        }
        this.a(ahb2, ajn.a, 0, 6, 1, 5, asv2);
        this.a(ahb2, ajn.a, 0, 6, 2, 5, asv2);
        this.a(ahb2, ajn.aa, 0, 6, 3, 4, asv2);
        this.a(ahb2, asv2, random, 6, 1, 5, this.a(ajn.ao, 1));
        for (n3 = 0; n3 < 5; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.b(ahb2, n2, 7, n3, asv2);
                this.b(ahb2, ajn.e, 0, n2, -1, n3, asv2);
            }
        }
        this.a(ahb2, asv2, 4, 1, 2, 2);
        return true;
    }

    @Override
    protected int b(int n2) {
        if (n2 == 0) {
            return 4;
        }
        return 0;
    }
}

