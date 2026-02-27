/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avy
extends awd {
    public avy() {
    }

    public avy(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static avy a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 5, 12, 9, n5);
        if (!avy.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avy(awa2, n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 12 - 1, 0);
        }
        this.a(ahb2, asv2, 1, 1, 1, 3, 3, 7, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 5, 1, 3, 9, 3, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 0, 0, 3, 0, 8, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 1, 0, 3, 10, 0, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 1, 1, 0, 10, 3, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 4, 1, 1, 4, 10, 3, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 0, 4, 0, 4, 7, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 4, 0, 4, 4, 4, 7, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 1, 8, 3, 4, 8, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 5, 4, 3, 10, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 5, 5, 3, 5, 7, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 9, 0, 4, 9, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 4, 0, 4, 4, 4, ajn.e, ajn.e, false);
        this.a(ahb2, ajn.e, 0, 0, 11, 2, asv2);
        this.a(ahb2, ajn.e, 0, 4, 11, 2, asv2);
        this.a(ahb2, ajn.e, 0, 2, 11, 0, asv2);
        this.a(ahb2, ajn.e, 0, 2, 11, 4, asv2);
        this.a(ahb2, ajn.e, 0, 1, 1, 6, asv2);
        this.a(ahb2, ajn.e, 0, 1, 1, 7, asv2);
        this.a(ahb2, ajn.e, 0, 2, 1, 7, asv2);
        this.a(ahb2, ajn.e, 0, 3, 1, 6, asv2);
        this.a(ahb2, ajn.e, 0, 3, 1, 7, asv2);
        this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 1, 1, 5, asv2);
        this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 2, 1, 6, asv2);
        this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 3, 1, 5, asv2);
        this.a(ahb2, ajn.ar, this.a(ajn.ar, 1), 1, 2, 7, asv2);
        this.a(ahb2, ajn.ar, this.a(ajn.ar, 0), 3, 2, 7, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 3, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 3, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 6, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 7, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 6, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 7, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 6, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 7, 0, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 6, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 7, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 0, 3, 6, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 3, 6, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 3, 8, asv2);
        this.a(ahb2, ajn.aa, 0, 2, 4, 7, asv2);
        this.a(ahb2, ajn.aa, 0, 1, 4, 6, asv2);
        this.a(ahb2, ajn.aa, 0, 3, 4, 6, asv2);
        this.a(ahb2, ajn.aa, 0, 2, 4, 5, asv2);
        int n3 = this.a(ajn.ap, 4);
        for (n2 = 1; n2 <= 9; ++n2) {
            this.a(ahb2, ajn.ap, n3, 3, n2, 3, asv2);
        }
        this.a(ahb2, ajn.a, 0, 2, 1, 0, asv2);
        this.a(ahb2, ajn.a, 0, 2, 2, 0, asv2);
        this.a(ahb2, asv2, random, 2, 1, 0, this.a(ajn.ao, 1));
        if (this.a(ahb2, 2, 0, -1, asv2).o() == awt.a && this.a(ahb2, 2, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 2, 0, -1, asv2);
        }
        for (n2 = 0; n2 < 9; ++n2) {
            for (int i2 = 0; i2 < 5; ++i2) {
                this.b(ahb2, i2, 12, n2, asv2);
                this.b(ahb2, ajn.e, 0, i2, -1, n2, asv2);
            }
        }
        this.a(ahb2, asv2, 2, 1, 2, 1);
        return true;
    }

    @Override
    protected int b(int n2) {
        return 2;
    }
}

