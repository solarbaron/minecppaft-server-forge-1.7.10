/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aus
extends avc {
    private boolean a;

    public aus() {
    }

    public aus(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Mob", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.n("Mob");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        if (avk2 != null) {
            ((auz)avk2).b = this;
        }
    }

    public static aus a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -4, -1, 0, 11, 8, 16, n5);
        if (!aus.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new aus(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        this.a(ahb2, asv2, 0, 0, 0, 10, 7, 15, false, random, aui.c());
        this.a(ahb2, random, asv2, avd.c, 4, 1, 0);
        int n4 = 6;
        this.a(ahb2, asv2, 1, n4, 1, 1, n4, 14, false, random, aui.c());
        this.a(ahb2, asv2, 9, n4, 1, 9, n4, 14, false, random, aui.c());
        this.a(ahb2, asv2, 2, n4, 1, 8, n4, 2, false, random, aui.c());
        this.a(ahb2, asv2, 2, n4, 14, 8, n4, 14, false, random, aui.c());
        this.a(ahb2, asv2, 1, 1, 1, 2, 1, 4, false, random, aui.c());
        this.a(ahb2, asv2, 8, 1, 1, 9, 1, 4, false, random, aui.c());
        this.a(ahb2, asv2, 1, 1, 1, 1, 1, 3, ajn.k, ajn.k, false);
        this.a(ahb2, asv2, 9, 1, 1, 9, 1, 3, ajn.k, ajn.k, false);
        this.a(ahb2, asv2, 3, 1, 8, 7, 1, 12, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 9, 6, 1, 11, ajn.k, ajn.k, false);
        for (n3 = 3; n3 < 14; n3 += 2) {
            this.a(ahb2, asv2, 0, 3, n3, 0, 4, n3, ajn.aY, ajn.aY, false);
            this.a(ahb2, asv2, 10, 3, n3, 10, 4, n3, ajn.aY, ajn.aY, false);
        }
        for (n3 = 2; n3 < 9; n3 += 2) {
            this.a(ahb2, asv2, n3, 3, 15, n3, 4, 15, ajn.aY, ajn.aY, false);
        }
        n3 = this.a(ajn.bg, 3);
        this.a(ahb2, asv2, 4, 1, 5, 6, 1, 7, false, random, aui.c());
        this.a(ahb2, asv2, 4, 2, 6, 6, 2, 7, false, random, aui.c());
        this.a(ahb2, asv2, 4, 3, 7, 6, 3, 7, false, random, aui.c());
        for (n2 = 4; n2 <= 6; ++n2) {
            this.a(ahb2, ajn.bg, n3, n2, 1, 4, asv2);
            this.a(ahb2, ajn.bg, n3, n2, 2, 5, asv2);
            this.a(ahb2, ajn.bg, n3, n2, 3, 6, asv2);
        }
        n2 = 2;
        int n5 = 0;
        int n6 = 3;
        int n7 = 1;
        switch (this.g) {
            case 0: {
                n2 = 0;
                n5 = 2;
                break;
            }
            case 3: {
                n2 = 3;
                n5 = 1;
                n6 = 0;
                n7 = 2;
                break;
            }
            case 1: {
                n2 = 1;
                n5 = 3;
                n6 = 0;
                n7 = 2;
            }
        }
        this.a(ahb2, ajn.br, n2 + (random.nextFloat() > 0.9f ? 4 : 0), 4, 3, 8, asv2);
        this.a(ahb2, ajn.br, n2 + (random.nextFloat() > 0.9f ? 4 : 0), 5, 3, 8, asv2);
        this.a(ahb2, ajn.br, n2 + (random.nextFloat() > 0.9f ? 4 : 0), 6, 3, 8, asv2);
        this.a(ahb2, ajn.br, n5 + (random.nextFloat() > 0.9f ? 4 : 0), 4, 3, 12, asv2);
        this.a(ahb2, ajn.br, n5 + (random.nextFloat() > 0.9f ? 4 : 0), 5, 3, 12, asv2);
        this.a(ahb2, ajn.br, n5 + (random.nextFloat() > 0.9f ? 4 : 0), 6, 3, 12, asv2);
        this.a(ahb2, ajn.br, n6 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 9, asv2);
        this.a(ahb2, ajn.br, n6 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 10, asv2);
        this.a(ahb2, ajn.br, n6 + (random.nextFloat() > 0.9f ? 4 : 0), 3, 3, 11, asv2);
        this.a(ahb2, ajn.br, n7 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 9, asv2);
        this.a(ahb2, ajn.br, n7 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 10, asv2);
        this.a(ahb2, ajn.br, n7 + (random.nextFloat() > 0.9f ? 4 : 0), 7, 3, 11, asv2);
        if (!this.a) {
            int n8;
            n4 = this.a(3);
            int n9 = this.a(5, 6);
            if (asv2.b(n9, n4, n8 = this.b(5, 6))) {
                this.a = true;
                ahb2.d(n9, n4, n8, ajn.ac, 0, 2);
                apj apj2 = (apj)ahb2.o(n9, n4, n8);
                if (apj2 != null) {
                    apj2.a().a("Silverfish");
                }
            }
        }
        return true;
    }
}

