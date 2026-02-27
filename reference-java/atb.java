/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atb
extends avk {
    public atb() {
    }

    public atb(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
    }

    @Override
    protected void a(dh dh2) {
    }

    @Override
    protected void b(dh dh2) {
    }

    public static asv a(List list, Random random, int n2, int n3, int n4, int n5) {
        asv asv2 = new asv(n2, n3 - 5, n4, n2, n3 + 2, n4);
        switch (n5) {
            case 2: {
                asv2.d = n2 + 2;
                asv2.c = n4 - 8;
                break;
            }
            case 0: {
                asv2.d = n2 + 2;
                asv2.f = n4 + 8;
                break;
            }
            case 1: {
                asv2.a = n2 - 8;
                asv2.f = n4 + 2;
                break;
            }
            case 3: {
                asv2.d = n2 + 8;
                asv2.f = n4 + 2;
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
        switch (this.g) {
            case 2: {
                asx.a(avk2, list, random, this.f.a, this.f.b, this.f.c - 1, 2, n2);
                break;
            }
            case 0: {
                asx.a(avk2, list, random, this.f.a, this.f.b, this.f.f + 1, 0, n2);
                break;
            }
            case 1: {
                asx.a(avk2, list, random, this.f.a - 1, this.f.b, this.f.c, 1, n2);
                break;
            }
            case 3: {
                asx.a(avk2, list, random, this.f.d + 1, this.f.b, this.f.c, 3, n2);
            }
        }
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 5, 0, 2, 7, 1, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 0, 7, 2, 2, 8, ajn.a, ajn.a, false);
        for (int i2 = 0; i2 < 5; ++i2) {
            this.a(ahb2, asv2, 0, 5 - i2 - (i2 < 4 ? 1 : 0), 2 + i2, 2, 7 - i2, 2 + i2, ajn.a, ajn.a, false);
        }
        return true;
    }
}

