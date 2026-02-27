/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avs
extends awd {
    private aji a;
    private aji b;

    public avs() {
    }

    public avs(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
        this.a = this.a(random);
        this.b = this.a(random);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("CA", aji.c.b(this.a));
        dh2.a("CB", aji.c.b(this.b));
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = aji.e(dh2.f("CA"));
        this.b = aji.e(dh2.f("CB"));
    }

    private aji a(Random random) {
        switch (random.nextInt(5)) {
            default: {
                return ajn.aj;
            }
            case 0: {
                return ajn.bM;
            }
            case 1: 
        }
        return ajn.bN;
    }

    public static avs a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 7, 4, 9, n5);
        if (!avs.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avs(awa2, n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 4 - 1, 0);
        }
        this.a(ahb2, asv2, 0, 1, 0, 6, 4, 8, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 0, 1, 2, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 4, 0, 1, 5, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 0, 0, 0, 0, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 6, 0, 0, 6, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 0, 0, 5, 0, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 0, 8, 5, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 3, 0, 1, 3, 0, 7, ajn.j, ajn.j, false);
        for (n2 = 1; n2 <= 7; ++n2) {
            this.a(ahb2, this.a, qh.a(random, 2, 7), 1, 1, n2, asv2);
            this.a(ahb2, this.a, qh.a(random, 2, 7), 2, 1, n2, asv2);
            this.a(ahb2, this.b, qh.a(random, 2, 7), 4, 1, n2, asv2);
            this.a(ahb2, this.b, qh.a(random, 2, 7), 5, 1, n2, asv2);
        }
        for (n2 = 0; n2 < 9; ++n2) {
            for (int i2 = 0; i2 < 7; ++i2) {
                this.b(ahb2, i2, 4, n2, asv2);
                this.b(ahb2, ajn.d, 0, i2, -1, n2, asv2);
            }
        }
        return true;
    }
}

