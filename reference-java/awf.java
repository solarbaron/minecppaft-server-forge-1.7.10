/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class awf
extends awd {
    public awf() {
    }

    public awf(awa awa2, int n2, Random random, int n3, int n4) {
        super(awa2, n2);
        this.g = random.nextInt(4);
        switch (this.g) {
            case 0: 
            case 2: {
                this.f = new asv(n3, 64, n4, n3 + 6 - 1, 78, n4 + 6 - 1);
                break;
            }
            default: {
                this.f = new asv(n3, 64, n4, n3 + 6 - 1, 78, n4 + 6 - 1);
            }
        }
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        avp.b((awa)avk2, list, random, this.f.a - 1, this.f.e - 4, this.f.c + 1, 1, this.d());
        avp.b((awa)avk2, list, random, this.f.d + 1, this.f.e - 4, this.f.c + 1, 3, this.d());
        avp.b((awa)avk2, list, random, this.f.a + 1, this.f.e - 4, this.f.c - 1, 2, this.d());
        avp.b((awa)avk2, list, random, this.f.a + 1, this.f.e - 4, this.f.f + 1, 0, this.d());
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 3, 0);
        }
        this.a(ahb2, asv2, 1, 0, 1, 4, 12, 4, ajn.e, ajn.i, false);
        this.a(ahb2, ajn.a, 0, 2, 12, 2, asv2);
        this.a(ahb2, ajn.a, 0, 3, 12, 2, asv2);
        this.a(ahb2, ajn.a, 0, 2, 12, 3, asv2);
        this.a(ahb2, ajn.a, 0, 3, 12, 3, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 13, 1, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 14, 1, asv2);
        this.a(ahb2, ajn.aJ, 0, 4, 13, 1, asv2);
        this.a(ahb2, ajn.aJ, 0, 4, 14, 1, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 13, 4, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 14, 4, asv2);
        this.a(ahb2, ajn.aJ, 0, 4, 13, 4, asv2);
        this.a(ahb2, ajn.aJ, 0, 4, 14, 4, asv2);
        this.a(ahb2, asv2, 1, 15, 1, 4, 15, 4, ajn.e, ajn.e, false);
        for (int i2 = 0; i2 <= 5; ++i2) {
            for (int i3 = 0; i3 <= 5; ++i3) {
                if (i3 != 0 && i3 != 5 && i2 != 0 && i2 != 5) continue;
                this.a(ahb2, ajn.n, 0, i3, 11, i2, asv2);
                this.b(ahb2, i3, 12, i2, asv2);
            }
        }
        return true;
    }
}

