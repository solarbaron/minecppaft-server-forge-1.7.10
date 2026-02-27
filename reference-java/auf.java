/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class auf
extends aue {
    private boolean e;

    public auf() {
    }

    public auf(Random random, int n2, int n3) {
        super(random, n2, 64, n3, 7, 5, 9);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Witch", this.e);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.e = dh2.n("Witch");
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        int n2;
        int n3;
        int n4;
        if (!this.a(ahb2, asv2, 0)) {
            return false;
        }
        this.a(ahb2, asv2, 1, 1, 1, 5, 1, 7, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 1, 4, 2, 5, 4, 7, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 2, 1, 0, 4, 1, 0, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 2, 2, 2, 3, 3, 2, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 1, 2, 3, 1, 3, 6, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 5, 2, 3, 5, 3, 6, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 2, 2, 7, 4, 3, 7, ajn.f, 1, ajn.f, 1, false);
        this.a(ahb2, asv2, 1, 0, 2, 1, 3, 2, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 5, 0, 2, 5, 3, 2, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 1, 0, 7, 1, 3, 7, ajn.r, ajn.r, false);
        this.a(ahb2, asv2, 5, 0, 7, 5, 3, 7, ajn.r, ajn.r, false);
        this.a(ahb2, ajn.aJ, 0, 2, 3, 2, asv2);
        this.a(ahb2, ajn.aJ, 0, 3, 3, 7, asv2);
        this.a(ahb2, ajn.a, 0, 1, 3, 4, asv2);
        this.a(ahb2, ajn.a, 0, 5, 3, 4, asv2);
        this.a(ahb2, ajn.a, 0, 5, 3, 5, asv2);
        this.a(ahb2, ajn.bL, 7, 1, 3, 5, asv2);
        this.a(ahb2, ajn.ai, 0, 3, 2, 6, asv2);
        this.a(ahb2, ajn.bp, 0, 4, 2, 6, asv2);
        this.a(ahb2, ajn.aJ, 0, 1, 2, 1, asv2);
        this.a(ahb2, ajn.aJ, 0, 5, 2, 1, asv2);
        int n5 = this.a(ajn.ad, 3);
        int n6 = this.a(ajn.ad, 1);
        int n7 = this.a(ajn.ad, 0);
        int n8 = this.a(ajn.ad, 2);
        this.a(ahb2, asv2, 0, 4, 1, 6, 4, 1, ajn.bF, n5, ajn.bF, n5, false);
        this.a(ahb2, asv2, 0, 4, 2, 0, 4, 7, ajn.bF, n7, ajn.bF, n7, false);
        this.a(ahb2, asv2, 6, 4, 2, 6, 4, 7, ajn.bF, n6, ajn.bF, n6, false);
        this.a(ahb2, asv2, 0, 4, 8, 6, 4, 8, ajn.bF, n8, ajn.bF, n8, false);
        for (n4 = 2; n4 <= 7; n4 += 5) {
            for (n3 = 1; n3 <= 5; n3 += 4) {
                this.b(ahb2, ajn.r, 0, n3, -1, n4, asv2);
            }
        }
        if (!this.e && asv2.b(n4 = this.a(2, 5), n3 = this.a(2), n2 = this.b(2, 5))) {
            this.e = true;
            yp yp2 = new yp(ahb2);
            yp2.b((double)n4 + 0.5, n3, (double)n2 + 0.5, 0.0f, 0.0f);
            yp2.a((sy)null);
            ahb2.d(yp2);
        }
        return true;
    }
}

