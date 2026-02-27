/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avx
extends awd {
    private boolean a;
    private int b;

    public avx() {
    }

    public avx(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
        this.a = random.nextBoolean();
        this.b = random.nextInt(3);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("T", this.b);
        dh2.a("C", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.b = dh2.f("T");
        this.a = dh2.n("C");
    }

    public static avx a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 4, 6, 5, n5);
        if (!avx.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new avx(awa2, n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 6 - 1, 0);
        }
        this.a(ahb2, asv2, 1, 1, 1, 3, 5, 4, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 0, 0, 0, 3, 0, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 1, 0, 1, 2, 0, 3, ajn.d, ajn.d, false);
        if (this.a) {
            this.a(ahb2, asv2, 1, 4, 1, 2, 4, 3, ajn.r, ajn.r, false);
        } else {
            this.a(ahb2, asv2, 1, 5, 1, 2, 5, 3, ajn.r, ajn.r, false);
        }
        this.a(ahb2, ajn.r, 0, 1, 4, 0, asv2);
        this.a(ahb2, ajn.r, 0, 2, 4, 0, asv2);
        this.a(ahb2, ajn.r, 0, 1, 4, 4, asv2);
        this.a(ahb2, ajn.r, 0, 2, 4, 4, asv2);
        this.a(ahb2, ajn.r, 0, 0, 4, 1, asv2);
        this.a(ahb2, ajn.r, 0, 0, 4, 2, asv2);
        this.a(ahb2, ajn.r, 0, 0, 4, 3, asv2);
        this.a(ahb2, ajn.r, 0, 3, 4, 1, asv2);
        this.a(ahb2, ajn.r, 0, 3, 4, 2, asv2);
        this.a(ahb2, ajn.r, 0, 3, 4, 3, asv2);
        this.a(ahb2, asv2, 0, 1, 0, 0, 3, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 3, 1, 0, 3, 3, 0, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 0, 1, 4, 0, 3, 4, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 3, 1, 4, 3, 3, 4, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 0, 1, 1, 0, 3, 3, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 3, 1, 1, 3, 3, 3, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 1, 0, 2, 3, 0, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 1, 4, 2, 3, 4, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 3, 2, 2, asv2);
        if (this.b > 0) {
            this.a(ahb2, ajn.aJ, 0, this.b, 1, 3, asv2);
            this.a(ahb2, ajn.aw, 0, this.b, 2, 3, asv2);
        }
        this.a(ahb2, ajn.a, 0, 1, 1, 0, asv2);
        this.a(ahb2, ajn.a, 0, 1, 2, 0, asv2);
        this.a(ahb2, asv2, random, 1, 1, 0, this.a(ajn.ao, 1));
        if (this.a(ahb2, 1, 0, -1, asv2).o() == awt.a && this.a(ahb2, 1, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 1, 0, -1, asv2);
        }
        for (int i2 = 0; i2 < 5; ++i2) {
            for (int i3 = 0; i3 < 4; ++i3) {
                this.b(ahb2, i3, 6, i2, asv2);
                this.b(ahb2, ajn.e, 0, i3, -1, i2, asv2);
            }
        }
        this.a(ahb2, asv2, 1, 1, 2, 1);
        return true;
    }
}

