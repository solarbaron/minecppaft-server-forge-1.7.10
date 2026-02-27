/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avt
extends awd {
    public avt() {
    }

    public avt(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
    }

    public static asv a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 3, 4, 2, n5);
        if (avk.a(list, asv2) != null) {
            return null;
        }
        return asv2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 4 - 1, 0);
        }
        this.a(ahb2, asv2, 0, 0, 0, 2, 3, 1, ajn.a, ajn.a, false);
        this.a(ahb2, ajn.aJ, 0, 1, 0, 0, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 1, 0, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 2, 0, asv2);
        this.a(ahb2, ajn.L, 15, 1, 3, 0, asv2);
        this.a(ahb2, ajn.aa, 0, 0, 3, 0, asv2);
        this.a(ahb2, ajn.aa, 0, 1, 3, 1, asv2);
        this.a(ahb2, ajn.aa, 0, 2, 3, 0, asv2);
        this.a(ahb2, ajn.aa, 0, 1, 3, -1, asv2);
        return true;
    }
}

