/*
 * Decompiled with CFR 0.152.
 */
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

public class ata
extends avk {
    private List a = new LinkedList();

    public ata() {
    }

    public ata(int n2, Random random, int n3, int n4) {
        super(n2);
        this.f = new asv(n3, 50, n4, n3 + 7 + random.nextInt(6), 54 + random.nextInt(6), n4 + 7 + random.nextInt(6));
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        asv asv2;
        avk avk3;
        int n2;
        int n3 = this.d();
        int n4 = this.f.c() - 3 - 1;
        if (n4 <= 0) {
            n4 = 1;
        }
        for (n2 = 0; n2 < this.f.b() && (n2 += random.nextInt(this.f.b())) + 3 <= this.f.b(); n2 += 4) {
            avk3 = asx.a(avk2, list, random, this.f.a + n2, this.f.b + random.nextInt(n4) + 1, this.f.c - 1, 2, n3);
            if (avk3 == null) continue;
            asv2 = avk3.c();
            this.a.add(new asv(asv2.a, asv2.b, this.f.c, asv2.d, asv2.e, this.f.c + 1));
        }
        for (n2 = 0; n2 < this.f.b() && (n2 += random.nextInt(this.f.b())) + 3 <= this.f.b(); n2 += 4) {
            avk3 = asx.a(avk2, list, random, this.f.a + n2, this.f.b + random.nextInt(n4) + 1, this.f.f + 1, 0, n3);
            if (avk3 == null) continue;
            asv2 = avk3.c();
            this.a.add(new asv(asv2.a, asv2.b, this.f.f - 1, asv2.d, asv2.e, this.f.f));
        }
        for (n2 = 0; n2 < this.f.d() && (n2 += random.nextInt(this.f.d())) + 3 <= this.f.d(); n2 += 4) {
            avk3 = asx.a(avk2, list, random, this.f.a - 1, this.f.b + random.nextInt(n4) + 1, this.f.c + n2, 1, n3);
            if (avk3 == null) continue;
            asv2 = avk3.c();
            this.a.add(new asv(this.f.a, asv2.b, asv2.c, this.f.a + 1, asv2.e, asv2.f));
        }
        for (n2 = 0; n2 < this.f.d() && (n2 += random.nextInt(this.f.d())) + 3 <= this.f.d(); n2 += 4) {
            avk3 = asx.a(avk2, list, random, this.f.d + 1, this.f.b + random.nextInt(n4) + 1, this.f.c + n2, 3, n3);
            if (avk3 == null) continue;
            asv2 = avk3.c();
            this.a.add(new asv(this.f.d - 1, asv2.b, asv2.c, this.f.d, asv2.e, asv2.f));
        }
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, this.f.a, this.f.b, this.f.c, this.f.d, this.f.b, this.f.f, ajn.d, ajn.a, true);
        this.a(ahb2, asv2, this.f.a, this.f.b + 1, this.f.c, this.f.d, Math.min(this.f.b + 3, this.f.e), this.f.f, ajn.a, ajn.a, false);
        for (asv asv3 : this.a) {
            this.a(ahb2, asv2, asv3.a, asv3.e - 2, asv3.c, asv3.d, asv3.e, asv3.f, ajn.a, ajn.a, false);
        }
        this.a(ahb2, asv2, this.f.a, this.f.b + 4, this.f.c, this.f.d, this.f.e, this.f.f, ajn.a, false);
        return true;
    }

    @Override
    protected void a(dh dh2) {
        dq dq2 = new dq();
        for (asv asv2 : this.a) {
            dq2.a(asv2.h());
        }
        dh2.a("Entrances", dq2);
    }

    @Override
    protected void b(dh dh2) {
        dq dq2 = dh2.c("Entrances", 11);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            this.a.add(new asv(dq2.c(i2)));
        }
    }
}

