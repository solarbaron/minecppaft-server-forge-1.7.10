/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aun
extends avc {
    private int a;

    public aun() {
    }

    public aun(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.f = asv2;
        this.a = n3 == 2 || n3 == 0 ? asv2.d() : asv2.b();
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Steps", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.f("Steps");
    }

    public static asv a(List list, Random random, int n2, int n3, int n4, int n5) {
        int n6 = 3;
        asv asv2 = asv.a(n2, n3, n4, -1, -1, 0, 5, 5, 4, n5);
        avk avk2 = avk.a(list, asv2);
        if (avk2 == null) {
            return null;
        }
        if (avk2.c().b == asv2.b) {
            for (int i2 = 3; i2 >= 1; --i2) {
                asv2 = asv.a(n2, n3, n4, -1, -1, 0, 5, 5, i2 - 1, n5);
                if (avk2.c().a(asv2)) continue;
                return asv.a(n2, n3, n4, -1, -1, 0, 5, 5, i2, n5);
            }
        }
        return null;
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        for (int i2 = 0; i2 < this.a; ++i2) {
            this.a(ahb2, ajn.aV, 0, 0, 0, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 1, 0, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 2, 0, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 3, 0, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 4, 0, i2, asv2);
            for (int i3 = 1; i3 <= 3; ++i3) {
                this.a(ahb2, ajn.aV, 0, 0, i3, i2, asv2);
                this.a(ahb2, ajn.a, 0, 1, i3, i2, asv2);
                this.a(ahb2, ajn.a, 0, 2, i3, i2, asv2);
                this.a(ahb2, ajn.a, 0, 3, i3, i2, asv2);
                this.a(ahb2, ajn.aV, 0, 4, i3, i2, asv2);
            }
            this.a(ahb2, ajn.aV, 0, 0, 4, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 1, 4, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 2, 4, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 3, 4, i2, asv2);
            this.a(ahb2, ajn.aV, 0, 4, 4, i2, asv2);
        }
        return true;
    }
}

