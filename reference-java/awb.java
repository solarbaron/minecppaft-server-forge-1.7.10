/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class awb
extends awe {
    private int a;

    public awb() {
    }

    public awb(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
        this.a = Math.max(asv2.b(), asv2.d());
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Length", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.f("Length");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        avk avk3;
        int n2;
        boolean bl2 = false;
        for (n2 = random.nextInt(5); n2 < this.a - 8; n2 += 2 + random.nextInt(5)) {
            avk3 = this.a((awa)avk2, list, random, 0, n2);
            if (avk3 == null) continue;
            n2 += Math.max(avk3.f.b(), avk3.f.d());
            bl2 = true;
        }
        for (n2 = random.nextInt(5); n2 < this.a - 8; n2 += 2 + random.nextInt(5)) {
            avk3 = this.b((awa)avk2, list, random, 0, n2);
            if (avk3 == null) continue;
            n2 += Math.max(avk3.f.b(), avk3.f.d());
            bl2 = true;
        }
        if (bl2 && random.nextInt(3) > 0) {
            switch (this.g) {
                case 2: {
                    avp.b((awa)avk2, list, random, this.f.a - 1, this.f.b, this.f.c, 1, this.d());
                    break;
                }
                case 0: {
                    avp.b((awa)avk2, list, random, this.f.a - 1, this.f.b, this.f.f - 2, 1, this.d());
                    break;
                }
                case 3: {
                    avp.b((awa)avk2, list, random, this.f.d - 2, this.f.b, this.f.c - 1, 2, this.d());
                    break;
                }
                case 1: {
                    avp.b((awa)avk2, list, random, this.f.a, this.f.b, this.f.c - 1, 2, this.d());
                }
            }
        }
        if (bl2 && random.nextInt(3) > 0) {
            switch (this.g) {
                case 2: {
                    avp.b((awa)avk2, list, random, this.f.d + 1, this.f.b, this.f.c, 3, this.d());
                    break;
                }
                case 0: {
                    avp.b((awa)avk2, list, random, this.f.d + 1, this.f.b, this.f.f - 2, 3, this.d());
                    break;
                }
                case 3: {
                    avp.b((awa)avk2, list, random, this.f.d - 2, this.f.b, this.f.f + 1, 0, this.d());
                    break;
                }
                case 1: {
                    avp.b((awa)avk2, list, random, this.f.a, this.f.b, this.f.f + 1, 0, this.d());
                }
            }
        }
    }

    public static asv a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5) {
        for (int i2 = 7 * qh.a(random, 3, 5); i2 >= 7; i2 -= 7) {
            asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 3, 3, i2, n5);
            if (avk.a(list, asv2) != null) continue;
            return asv2;
        }
        return null;
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        aji aji2 = this.b(ajn.n, 0);
        for (int i2 = this.f.a; i2 <= this.f.d; ++i2) {
            for (int i3 = this.f.c; i3 <= this.f.f; ++i3) {
                if (!asv2.b(i2, 64, i3)) continue;
                int n2 = ahb2.i(i2, i3) - 1;
                ahb2.d(i2, n2, i3, aji2, 0, 2);
            }
        }
        return true;
    }
}

