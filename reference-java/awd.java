/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

abstract class awd
extends avk {
    protected int k = -1;
    private int a;
    private boolean b;

    public awd() {
    }

    protected awd(awa awa2, int n2) {
        super(n2);
        if (awa2 != null) {
            this.b = awa2.b;
        }
    }

    @Override
    protected void a(dh dh2) {
        dh2.a("HPos", this.k);
        dh2.a("VCount", this.a);
        dh2.a("Desert", this.b);
    }

    @Override
    protected void b(dh dh2) {
        this.k = dh2.f("HPos");
        this.a = dh2.f("VCount");
        this.b = dh2.n("Desert");
    }

    protected avk a(awa awa2, List list, Random random, int n2, int n3) {
        switch (this.g) {
            case 2: {
                return avp.a(awa2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d());
            }
            case 0: {
                return avp.a(awa2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d());
            }
            case 1: {
                return avp.a(awa2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d());
            }
            case 3: {
                return avp.a(awa2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d());
            }
        }
        return null;
    }

    protected avk b(awa awa2, List list, Random random, int n2, int n3) {
        switch (this.g) {
            case 2: {
                return avp.a(awa2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d());
            }
            case 0: {
                return avp.a(awa2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d());
            }
            case 1: {
                return avp.a(awa2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d());
            }
            case 3: {
                return avp.a(awa2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d());
            }
        }
        return null;
    }

    protected int b(ahb ahb2, asv asv2) {
        int n2 = 0;
        int n3 = 0;
        for (int i2 = this.f.c; i2 <= this.f.f; ++i2) {
            for (int i3 = this.f.a; i3 <= this.f.d; ++i3) {
                if (!asv2.b(i3, 64, i2)) continue;
                n2 += Math.max(ahb2.i(i3, i2), ahb2.t.i());
                ++n3;
            }
        }
        if (n3 == 0) {
            return -1;
        }
        return n2 / n3;
    }

    protected static boolean a(asv asv2) {
        return asv2 != null && asv2.b > 10;
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5) {
        int n6;
        int n7;
        int n8;
        if (this.a >= n5) {
            return;
        }
        for (int i2 = this.a; i2 < n5 && asv2.b(n8 = this.a(n2 + i2, n4), n7 = this.a(n3), n6 = this.b(n2 + i2, n4)); ++i2) {
            ++this.a;
            yv yv2 = new yv(ahb2, this.b(i2));
            yv2.b((double)n8 + 0.5, n7, (double)n6 + 0.5, 0.0f, 0.0f);
            ahb2.d(yv2);
        }
    }

    protected int b(int n2) {
        return 0;
    }

    protected aji b(aji aji2, int n2) {
        if (this.b) {
            if (aji2 == ajn.r || aji2 == ajn.s) {
                return ajn.A;
            }
            if (aji2 == ajn.e) {
                return ajn.A;
            }
            if (aji2 == ajn.f) {
                return ajn.A;
            }
            if (aji2 == ajn.ad) {
                return ajn.bz;
            }
            if (aji2 == ajn.ar) {
                return ajn.bz;
            }
            if (aji2 == ajn.n) {
                return ajn.A;
            }
        }
        return aji2;
    }

    protected int c(aji aji2, int n2) {
        if (this.b) {
            if (aji2 == ajn.r || aji2 == ajn.s) {
                return 0;
            }
            if (aji2 == ajn.e) {
                return 0;
            }
            if (aji2 == ajn.f) {
                return 2;
            }
        }
        return n2;
    }

    @Override
    protected void a(ahb ahb2, aji aji2, int n2, int n3, int n4, int n5, asv asv2) {
        aji aji3 = this.b(aji2, n2);
        int n6 = this.c(aji2, n2);
        super.a(ahb2, aji3, n6, n3, n4, n5, asv2);
    }

    @Override
    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7, aji aji2, aji aji3, boolean bl2) {
        aji aji4 = this.b(aji2, 0);
        int n8 = this.c(aji2, 0);
        aji aji5 = this.b(aji3, 0);
        int n9 = this.c(aji3, 0);
        super.a(ahb2, asv2, n2, n3, n4, n5, n6, n7, aji4, n8, aji5, n9, bl2);
    }

    @Override
    protected void b(ahb ahb2, aji aji2, int n2, int n3, int n4, int n5, asv asv2) {
        aji aji3 = this.b(aji2, n2);
        int n6 = this.c(aji2, n2);
        super.b(ahb2, aji3, n6, n3, n4, n5, asv2);
    }
}

