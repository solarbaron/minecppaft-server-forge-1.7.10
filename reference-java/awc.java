/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class awc
extends awd {
    public awc() {
    }

    public awc(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static awc a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 9, 7, 12, n5);
        if (!awc.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new awc(awa2, n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        int n4;
        int n5;
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 7 - 1, 0);
        }
        this.a(ahb2, asv2, 1, 1, 1, 7, 4, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 2, 1, 6, 8, 4, 10, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 2, 0, 5, 8, 0, 10, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 0, 1, 7, 0, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 0, 0, 0, 3, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 8, 0, 0, 8, 3, 10, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 0, 0, 7, 2, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 0, 5, 2, 1, 5, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 2, 0, 6, 2, 3, 10, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 3, 0, 10, 7, 3, 10, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 2, 0, 7, 3, 0, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 2, 5, 2, 3, 5, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 4, 1, 8, 4, 1, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 4, 4, 3, 4, 4, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 0, 5, 2, 8, 5, 3, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.f, 0, 0, 4, 2, asv2);
        this.a(ahb2, ajn.f, 0, 0, 4, 3, asv2);
        this.a(ahb2, ajn.f, 0, 8, 4, 2, asv2);
        this.a(ahb2, ajn.f, 0, 8, 4, 3, asv2);
        this.a(ahb2, ajn.f, 0, 8, 4, 4, asv2);
        int n6 = this.a(ajn.ad, 3);
        int n7 = this.a(ajn.ad, 2);
        for (n5 = -1; n5 <= 2; ++n5) {
            for (n4 = 0; n4 <= 8; ++n4) {
                this.a(ahb2, ajn.ad, n6, n4, 4 + n5, n5, asv2);
                if (n5 <= -1 && n4 > 1 || n5 <= 0 && n4 > 3 || n5 <= 1 && n4 > 4 && n4 < 6) continue;
                this.a(ahb2, ajn.ad, n7, n4, 4 + n5, 5 - n5, asv2);
            }
        }
        this.a(ahb2, asv2, 3, 4, 5, 3, 4, 10, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 7, 4, 2, 7, 4, 10, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 4, 5, 4, 4, 5, 10, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 6, 5, 4, 6, 5, 10, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 5, 6, 3, 5, 6, 10, ajn.f, ajn.f, false);
        n5 = this.a(ajn.ad, 0);
        for (n4 = 4; n4 >= 1; --n4) {
            this.a(ahb2, ajn.f, 0, n4, 2 + n4, 7 - n4, asv2);
            for (n3 = 8 - n4; n3 <= 10; ++n3) {
                this.a(ahb2, ajn.ad, n5, n4, 2 + n4, n3, asv2);
            }
        }
        n4 = this.a(ajn.ad, 1);
        this.a(ahb2, ajn.f, 0, 6, 6, 3, asv2);
        this.a(ahb2, ajn.f, 0, 7, 5, 4, asv2);
        this.a(ahb2, ajn.ad, n4, 6, 6, 4, asv2);
        for (n3 = 6; n3 <= 8; ++n3) {
            for (n2 = 5; n2 <= 10; ++n2) {
                this.a(ahb2, ajn.ad, n4, n3, 12 - n3, n2, asv2);
            }
        }
        this.a(ahb2, ajn.r, 0, 0, 2, 1, asv2);
        this.a(ahb2, ajn.r, 0, 0, 2, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 3, asv2);
        this.a(ahb2, ajn.r, 0, 4, 2, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 2, 0, asv2);
        this.a(ahb2, ajn.r, 0, 6, 2, 0, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 1, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 3, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 4, asv2);
        this.a(ahb2, ajn.f, 0, 8, 2, 5, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 6, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 7, asv2);
        this.a(ahb2, ajn.aZ, 0, 8, 2, 8, asv2);
        this.a(ahb2, ajn.r, 0, 8, 2, 9, asv2);
        this.a(ahb2, ajn.r, 0, 2, 2, 6, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 7, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 8, asv2);
        this.a(ahb2, ajn.r, 0, 2, 2, 9, asv2);
        this.a(ahb2, ajn.r, 0, 4, 4, 10, asv2);
        this.a(ahb2, ajn.aZ, 0, 5, 4, 10, asv2);
        this.a(ahb2, ajn.r, 0, 6, 4, 10, asv2);
        this.a(ahb2, ajn.f, 0, 5, 5, 10, asv2);
        this.a(ahb2, ajn.a, 0, 2, 1, 0, asv2);
        this.a(ahb2, ajn.a, 0, 2, 2, 0, asv2);
        this.a(ahb2, ajn.aa, 0, 2, 3, 1, asv2);
        this.a(ahb2, asv2, random, 2, 1, 0, this.a(ajn.ao, 1));
        this.a(ahb2, asv2, 1, 0, -1, 3, 2, -1, ajn.a, ajn.a, false);
        if (this.a(ahb2, 2, 0, -1, asv2).o() == awt.a && this.a(ahb2, 2, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 2, 0, -1, asv2);
        }
        for (n3 = 0; n3 < 5; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.b(ahb2, n2, 7, n3, asv2);
                this.b(ahb2, ajn.e, 0, n2, -1, n3, asv2);
            }
        }
        for (n3 = 5; n3 < 11; ++n3) {
            for (n2 = 2; n2 < 9; ++n2) {
                this.b(ahb2, n2, 7, n3, asv2);
                this.b(ahb2, ajn.e, 0, n2, -1, n3, asv2);
            }
        }
        this.a(ahb2, asv2, 4, 1, 2, 2);
        return true;
    }
}

