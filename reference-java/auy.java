/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auy
extends avc {
    private boolean a;

    public auy() {
    }

    public auy(int n2, Random random, int n3, int n4) {
        super(n2);
        this.a = true;
        this.g = random.nextInt(4);
        this.d = avd.a;
        switch (this.g) {
            case 0: 
            case 2: {
                this.f = new asv(n3, 64, n4, n3 + 5 - 1, 74, n4 + 5 - 1);
                break;
            }
            default: {
                this.f = new asv(n3, 64, n4, n3 + 5 - 1, 74, n4 + 5 - 1);
            }
        }
    }

    public auy(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.a = false;
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("Source", this.a);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.n("Source");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        if (this.a) {
            aui.a(auo.class);
        }
        this.a((auz)avk2, list, random, 1, 1);
    }

    public static auy a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -1, -7, 0, 5, 11, 5, n5);
        if (!auy.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new auy(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 4, 10, 4, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 1, 7, 0);
        this.a(ahb2, random, asv2, avd.a, 1, 1, 4);
        this.a(ahb2, ajn.aV, 0, 2, 6, 1, asv2);
        this.a(ahb2, ajn.aV, 0, 1, 5, 1, asv2);
        this.a(ahb2, ajn.U, 0, 1, 6, 1, asv2);
        this.a(ahb2, ajn.aV, 0, 1, 5, 2, asv2);
        this.a(ahb2, ajn.aV, 0, 1, 4, 3, asv2);
        this.a(ahb2, ajn.U, 0, 1, 5, 3, asv2);
        this.a(ahb2, ajn.aV, 0, 2, 4, 3, asv2);
        this.a(ahb2, ajn.aV, 0, 3, 3, 3, asv2);
        this.a(ahb2, ajn.U, 0, 3, 4, 3, asv2);
        this.a(ahb2, ajn.aV, 0, 3, 3, 2, asv2);
        this.a(ahb2, ajn.aV, 0, 3, 2, 1, asv2);
        this.a(ahb2, ajn.U, 0, 3, 3, 1, asv2);
        this.a(ahb2, ajn.aV, 0, 2, 2, 1, asv2);
        this.a(ahb2, ajn.aV, 0, 1, 1, 1, asv2);
        this.a(ahb2, ajn.U, 0, 1, 2, 1, asv2);
        this.a(ahb2, ajn.aV, 0, 1, 1, 2, asv2);
        this.a(ahb2, ajn.U, 0, 1, 1, 3, asv2);
        return true;
    }
}

