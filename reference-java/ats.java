/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

abstract class ats
extends avk {
    protected static final qx[] a = new qx[]{new qx(ade.i, 0, 1, 3, 5), new qx(ade.j, 0, 1, 5, 5), new qx(ade.k, 0, 1, 3, 15), new qx(ade.B, 0, 1, 1, 5), new qx(ade.ah, 0, 1, 1, 5), new qx(ade.d, 0, 1, 1, 5), new qx(ade.bm, 0, 3, 7, 5), new qx(ade.av, 0, 1, 1, 10), new qx(ade.bY, 0, 1, 1, 8), new qx(ade.bX, 0, 1, 1, 5), new qx(ade.bZ, 0, 1, 1, 3)};

    public ats() {
    }

    protected ats(int n2) {
        super(n2);
    }

    @Override
    protected void b(dh dh2) {
    }

    @Override
    protected void a(dh dh2) {
    }

    private int a(List list) {
        boolean bl2 = false;
        int n2 = 0;
        for (att att2 : list) {
            if (att2.d > 0 && att2.c < att2.d) {
                bl2 = true;
            }
            n2 += att2.b;
        }
        return bl2 ? n2 : -1;
    }

    private ats a(atw atw2, List list, List list2, Random random, int n2, int n3, int n4, int n5, int n6) {
        int n7 = this.a(list);
        boolean bl2 = n7 > 0 && n6 <= 30;
        int n8 = 0;
        block0: while (n8 < 5 && bl2) {
            ++n8;
            int n9 = random.nextInt(n7);
            for (att att2 : list) {
                if ((n9 -= att2.b) >= 0) continue;
                if (!att2.a(n6) || att2 == atw2.b && !att2.e) continue block0;
                ats ats2 = atf.a(att2, list2, random, n2, n3, n4, n5, n6);
                if (ats2 == null) continue;
                ++att2.c;
                atw2.b = att2;
                if (!att2.a()) {
                    list.remove(att2);
                }
                return ats2;
            }
        }
        return ath.a(list2, random, n2, n3, n4, n5, n6);
    }

    private avk a(atw atw2, List list, Random random, int n2, int n3, int n4, int n5, int n6, boolean bl2) {
        ats ats2;
        if (Math.abs(n2 - atw2.c().a) > 112 || Math.abs(n4 - atw2.c().c) > 112) {
            return ath.a(list, random, n2, n3, n4, n5, n6);
        }
        List list2 = atw2.c;
        if (bl2) {
            list2 = atw2.d;
        }
        if ((ats2 = this.a(atw2, list2, list, random, n2, n3, n4, n5, n6 + 1)) != null) {
            list.add(ats2);
            atw2.e.add(ats2);
        }
        return ats2;
    }

    protected avk a(atw atw2, List list, Random random, int n2, int n3, boolean bl2) {
        switch (this.g) {
            case 2: {
                return this.a(atw2, list, random, this.f.a + n2, this.f.b + n3, this.f.c - 1, this.g, this.d(), bl2);
            }
            case 0: {
                return this.a(atw2, list, random, this.f.a + n2, this.f.b + n3, this.f.f + 1, this.g, this.d(), bl2);
            }
            case 1: {
                return this.a(atw2, list, random, this.f.a - 1, this.f.b + n3, this.f.c + n2, this.g, this.d(), bl2);
            }
            case 3: {
                return this.a(atw2, list, random, this.f.d + 1, this.f.b + n3, this.f.c + n2, this.g, this.d(), bl2);
            }
        }
        return null;
    }

    protected avk b(atw atw2, List list, Random random, int n2, int n3, boolean bl2) {
        switch (this.g) {
            case 2: {
                return this.a(atw2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d(), bl2);
            }
            case 0: {
                return this.a(atw2, list, random, this.f.a - 1, this.f.b + n2, this.f.c + n3, 1, this.d(), bl2);
            }
            case 1: {
                return this.a(atw2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d(), bl2);
            }
            case 3: {
                return this.a(atw2, list, random, this.f.a + n3, this.f.b + n2, this.f.c - 1, 2, this.d(), bl2);
            }
        }
        return null;
    }

    protected avk c(atw atw2, List list, Random random, int n2, int n3, boolean bl2) {
        switch (this.g) {
            case 2: {
                return this.a(atw2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d(), bl2);
            }
            case 0: {
                return this.a(atw2, list, random, this.f.d + 1, this.f.b + n2, this.f.c + n3, 3, this.d(), bl2);
            }
            case 1: {
                return this.a(atw2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d(), bl2);
            }
            case 3: {
                return this.a(atw2, list, random, this.f.a + n3, this.f.b + n2, this.f.f + 1, 0, this.d(), bl2);
            }
        }
        return null;
    }

    protected static boolean a(asv asv2) {
        return asv2 != null && asv2.b > 10;
    }
}

