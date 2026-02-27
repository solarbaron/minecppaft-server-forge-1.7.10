/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class asz
extends avk {
    private int a;
    private boolean b;

    public asz() {
    }

    @Override
    protected void a(dh dh2) {
        dh2.a("tf", this.b);
        dh2.a("D", this.a);
    }

    @Override
    protected void b(dh dh2) {
        this.b = dh2.n("tf");
        this.a = dh2.f("D");
    }

    public asz(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.a = n3;
        this.f = asv2;
        this.b = asv2.c() > 3;
    }

    public static asv a(List list, Random random, int n2, int n3, int n4, int n5) {
        asv asv2 = new asv(n2, n3, n4, n2, n3 + 2, n4);
        if (random.nextInt(4) == 0) {
            asv2.e += 4;
        }
        switch (n5) {
            case 2: {
                asv2.a = n2 - 1;
                asv2.d = n2 + 3;
                asv2.c = n4 - 4;
                break;
            }
            case 0: {
                asv2.a = n2 - 1;
                asv2.d = n2 + 3;
                asv2.f = n4 + 4;
                break;
            }
            case 1: {
                asv2.a = n2 - 4;
                asv2.c = n4 - 1;
                asv2.f = n4 + 3;
                break;
            }
            case 3: {
                asv2.d = n2 + 4;
                asv2.c = n4 - 1;
                asv2.f = n4 + 3;
            }
        }
        if (avk.a(list, asv2) != null) {
            return null;
        }
        return asv2;
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        int n2 = this.d();
        switch (this.a) {
            case 2: {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.c - 1, 2, n2);
                asx.a(avk2, list, random, this.f.a - 1, this.f.b, this.f.c + 1, 1, n2);
                asx.a(avk2, list, random, this.f.d + 1, this.f.b, this.f.c + 1, 3, n2);
                break;
            }
            case 0: {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.f + 1, 0, n2);
                asx.a(avk2, list, random, this.f.a - 1, this.f.b, this.f.c + 1, 1, n2);
                asx.a(avk2, list, random, this.f.d + 1, this.f.b, this.f.c + 1, 3, n2);
                break;
            }
            case 1: {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.c - 1, 2, n2);
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.f + 1, 0, n2);
                asx.a(avk2, list, random, this.f.a - 1, this.f.b, this.f.c + 1, 1, n2);
                break;
            }
            case 3: {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.c - 1, 2, n2);
                asx.a(avk2, list, random, this.f.a + 1, this.f.b, this.f.f + 1, 0, n2);
                asx.a(avk2, list, random, this.f.d + 1, this.f.b, this.f.c + 1, 3, n2);
            }
        }
        if (this.b) {
            if (random.nextBoolean()) {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b + 3 + 1, this.f.c - 1, 2, n2);
            }
            if (random.nextBoolean()) {
                asx.a(avk2, list, random, this.f.a - 1, this.f.b + 3 + 1, this.f.c + 1, 1, n2);
            }
            if (random.nextBoolean()) {
                asx.a(avk2, list, random, this.f.d + 1, this.f.b + 3 + 1, this.f.c + 1, 3, n2);
            }
            if (random.nextBoolean()) {
                asx.a(avk2, list, random, this.f.a + 1, this.f.b + 3 + 1, this.f.f + 1, 0, n2);
            }
        }
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        if (this.b) {
            this.a(ahb2, asv2, this.f.a + 1, this.f.b, this.f.c, this.f.d - 1, this.f.b + 3 - 1, this.f.f, ajn.a, ajn.a, false);
            this.a(ahb2, asv2, this.f.a, this.f.b, this.f.c + 1, this.f.d, this.f.b + 3 - 1, this.f.f - 1, ajn.a, ajn.a, false);
            this.a(ahb2, asv2, this.f.a + 1, this.f.e - 2, this.f.c, this.f.d - 1, this.f.e, this.f.f, ajn.a, ajn.a, false);
            this.a(ahb2, asv2, this.f.a, this.f.e - 2, this.f.c + 1, this.f.d, this.f.e, this.f.f - 1, ajn.a, ajn.a, false);
            this.a(ahb2, asv2, this.f.a + 1, this.f.b + 3, this.f.c + 1, this.f.d - 1, this.f.b + 3, this.f.f - 1, ajn.a, ajn.a, false);
        } else {
            this.a(ahb2, asv2, this.f.a + 1, this.f.b, this.f.c, this.f.d - 1, this.f.e, this.f.f, ajn.a, ajn.a, false);
            this.a(ahb2, asv2, this.f.a, this.f.b, this.f.c + 1, this.f.d, this.f.e, this.f.f - 1, ajn.a, ajn.a, false);
        }
        this.a(ahb2, asv2, this.f.a + 1, this.f.b, this.f.c + 1, this.f.a + 1, this.f.e, this.f.c + 1, ajn.f, ajn.a, false);
        this.a(ahb2, asv2, this.f.a + 1, this.f.b, this.f.f - 1, this.f.a + 1, this.f.e, this.f.f - 1, ajn.f, ajn.a, false);
        this.a(ahb2, asv2, this.f.d - 1, this.f.b, this.f.c + 1, this.f.d - 1, this.f.e, this.f.c + 1, ajn.f, ajn.a, false);
        this.a(ahb2, asv2, this.f.d - 1, this.f.b, this.f.f - 1, this.f.d - 1, this.f.e, this.f.f - 1, ajn.f, ajn.a, false);
        for (int i2 = this.f.a; i2 <= this.f.d; ++i2) {
            for (int i3 = this.f.c; i3 <= this.f.f; ++i3) {
                if (this.a(ahb2, i2, this.f.b - 1, i3, asv2).o() != awt.a) continue;
                this.a(ahb2, ajn.f, 0, i2, this.f.b - 1, i3, asv2);
            }
        }
        return true;
    }
}

