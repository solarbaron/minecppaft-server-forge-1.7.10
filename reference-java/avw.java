/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class avw
extends awd {
    private boolean a;

    public avw() {
    }

    public avw(awa awa2, int n2, Random random, asv asv2, int n3) {
        super(awa2, n2);
        this.g = n3;
        this.f = asv2;
        this.a = random.nextBoolean();
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Terrace", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.n("Terrace");
    }

    public static avw a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, 0, 0, 0, 5, 6, 5, n5);
        if (avk.a(list, asv2) != null) {
            return null;
        }
        return new avw(awa2, n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        if (this.k < 0) {
            this.k = this.b(ahb2, asv2);
            if (this.k < 0) {
                return true;
            }
            this.f.a(0, this.k - this.f.e + 6 - 1, 0);
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 0, 4, ajn.e, ajn.e, false);
        this.a(ahb2, asv2, 0, 4, 0, 4, 4, 4, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 4, 1, 3, 4, 3, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.e, 0, 0, 1, 0, asv2);
        this.a(ahb2, ajn.e, 0, 0, 2, 0, asv2);
        this.a(ahb2, ajn.e, 0, 0, 3, 0, asv2);
        this.a(ahb2, ajn.e, 0, 4, 1, 0, asv2);
        this.a(ahb2, ajn.e, 0, 4, 2, 0, asv2);
        this.a(ahb2, ajn.e, 0, 4, 3, 0, asv2);
        this.a(ahb2, ajn.e, 0, 0, 1, 4, asv2);
        this.a(ahb2, ajn.e, 0, 0, 2, 4, asv2);
        this.a(ahb2, ajn.e, 0, 0, 3, 4, asv2);
        this.a(ahb2, ajn.e, 0, 4, 1, 4, asv2);
        this.a(ahb2, ajn.e, 0, 4, 2, 4, asv2);
        this.a(ahb2, ajn.e, 0, 4, 3, 4, asv2);
        this.a(ahb2, asv2, 0, 1, 1, 0, 3, 3, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 4, 1, 1, 4, 3, 3, ajn.f, ajn.f, false);
        this.a(ahb2, asv2, 1, 1, 4, 3, 3, 4, ajn.f, ajn.f, false);
        this.a(ahb2, ajn.aZ, 0, 0, 2, 2, asv2);
        this.a(ahb2, ajn.aZ, 0, 2, 2, 4, asv2);
        this.a(ahb2, ajn.aZ, 0, 4, 2, 2, asv2);
        this.a(ahb2, ajn.f, 0, 1, 1, 0, asv2);
        this.a(ahb2, ajn.f, 0, 1, 2, 0, asv2);
        this.a(ahb2, ajn.f, 0, 1, 3, 0, asv2);
        this.a(ahb2, ajn.f, 0, 2, 3, 0, asv2);
        this.a(ahb2, ajn.f, 0, 3, 3, 0, asv2);
        this.a(ahb2, ajn.f, 0, 3, 2, 0, asv2);
        this.a(ahb2, ajn.f, 0, 3, 1, 0, asv2);
        if (this.a(ahb2, 2, 0, -1, asv2).o() == awt.a && this.a(ahb2, 2, -1, -1, asv2).o() != awt.a) {
            this.a(ahb2, ajn.ar, this.a(ajn.ar, 3), 2, 0, -1, asv2);
        }
        this.a(ahb2, asv2, 1, 1, 1, 3, 3, 3, ajn.a, ajn.a, false);
        if (this.a) {
            this.a(ahb2, ajn.aJ, 0, 0, 5, 0, asv2);
            this.a(ahb2, ajn.aJ, 0, 1, 5, 0, asv2);
            this.a(ahb2, ajn.aJ, 0, 2, 5, 0, asv2);
            this.a(ahb2, ajn.aJ, 0, 3, 5, 0, asv2);
            this.a(ahb2, ajn.aJ, 0, 4, 5, 0, asv2);
            this.a(ahb2, ajn.aJ, 0, 0, 5, 4, asv2);
            this.a(ahb2, ajn.aJ, 0, 1, 5, 4, asv2);
            this.a(ahb2, ajn.aJ, 0, 2, 5, 4, asv2);
            this.a(ahb2, ajn.aJ, 0, 3, 5, 4, asv2);
            this.a(ahb2, ajn.aJ, 0, 4, 5, 4, asv2);
            this.a(ahb2, ajn.aJ, 0, 4, 5, 1, asv2);
            this.a(ahb2, ajn.aJ, 0, 4, 5, 2, asv2);
            this.a(ahb2, ajn.aJ, 0, 4, 5, 3, asv2);
            this.a(ahb2, ajn.aJ, 0, 0, 5, 1, asv2);
            this.a(ahb2, ajn.aJ, 0, 0, 5, 2, asv2);
            this.a(ahb2, ajn.aJ, 0, 0, 5, 3, asv2);
        }
        if (this.a) {
            n2 = this.a(ajn.ap, 3);
            this.a(ahb2, ajn.ap, n2, 3, 1, 3, asv2);
            this.a(ahb2, ajn.ap, n2, 3, 2, 3, asv2);
            this.a(ahb2, ajn.ap, n2, 3, 3, 3, asv2);
            this.a(ahb2, ajn.ap, n2, 3, 4, 3, asv2);
        }
        this.a(ahb2, ajn.aa, 0, 2, 3, 1, asv2);
        for (n2 = 0; n2 < 5; ++n2) {
            for (int i2 = 0; i2 < 5; ++i2) {
                this.b(ahb2, i2, 6, n2, asv2);
                this.b(ahb2, ajn.e, 0, i2, -1, n2, asv2);
            }
        }
        this.a(ahb2, asv2, 1, 1, 2, 1);
        return true;
    }
}

