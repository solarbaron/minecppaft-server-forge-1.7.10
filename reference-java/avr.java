/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avr
extends awd {
    private aji a;
    private aji b;
    private aji c;
    private aji d;

    public avr() {
    }

    public avr(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
        this.a = this.a(random);
        this.b = this.a(random);
        this.c = this.a(random);
        this.d = this.a(random);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("CA", aji.c.b(this.a));
        dh2.a("CB", aji.c.b(this.b));
        dh2.a("CC", aji.c.b(this.c));
        dh2.a("CD", aji.c.b(this.d));
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = aji.e(dh2.f("CA"));
        this.b = aji.e(dh2.f("CB"));
        this.c = aji.e(dh2.f("CC"));
        this.d = aji.e(dh2.f("CD"));
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

    public static avr a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 13, 4, 9, n5);
        if (!avr.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avr(awa2, n6, random, asv2, n5);
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
        this.a(ahb2, asv2, 0, 1, 0, 12, 4, 8, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 0, 1, 2, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 4, 0, 1, 5, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 7, 0, 1, 8, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 10, 0, 1, 11, 0, 7, ajn.ak, ajn.ak, false);
        this.a(ahb2, asv2, 0, 0, 0, 0, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 6, 0, 0, 6, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 12, 0, 0, 12, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 0, 0, 11, 0, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 0, 8, 11, 0, 8, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 3, 0, 1, 3, 0, 7, ajn.j, ajn.j, false);
        this.a(ahb2, asv2, 9, 0, 1, 9, 0, 7, ajn.j, ajn.j, false);
        for (n2 = 1; n2 <= 7; ++n2) {
            this.a(ahb2, this.a, qh.a(random, 2, 7), 1, 1, n2, asv2);
            this.a(ahb2, this.a, qh.a(random, 2, 7), 2, 1, n2, asv2);
            this.a(ahb2, this.b, qh.a(random, 2, 7), 4, 1, n2, asv2);
            this.a(ahb2, this.b, qh.a(random, 2, 7), 5, 1, n2, asv2);
            this.a(ahb2, this.c, qh.a(random, 2, 7), 7, 1, n2, asv2);
            this.a(ahb2, this.c, qh.a(random, 2, 7), 8, 1, n2, asv2);
            this.a(ahb2, this.d, qh.a(random, 2, 7), 10, 1, n2, asv2);
            this.a(ahb2, this.d, qh.a(random, 2, 7), 11, 1, n2, asv2);
        }
        for (n2 = 0; n2 < 9; ++n2) {
            for (int i2 = 0; i2 < 13; ++i2) {
                this.b(ahb2, i2, 4, n2, asv2);
                this.b(ahb2, ajn.d, 0, i2, -1, n2, asv2);
            }
        }
        return true;
    }
}

