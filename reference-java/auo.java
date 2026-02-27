/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class auo
extends avc {
    private boolean a;
    private boolean b;
    private boolean c;
    private boolean e;

    public auo() {
    }

    public auo(int n2, Random random, asv asv2, int n3) {
        super(n2);
        this.g = n3;
        this.d = this.a(random);
        this.f = asv2;
        this.a = random.nextBoolean();
        this.b = random.nextBoolean();
        this.c = random.nextBoolean();
        this.e = random.nextInt(3) > 0;
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dh2.a("leftLow", this.a);
        dh2.a("leftHigh", this.b);
        dh2.a("rightLow", this.c);
        dh2.a("rightHigh", this.e);
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        this.a = dh2.n("leftLow");
        this.b = dh2.n("leftHigh");
        this.c = dh2.n("rightLow");
        this.e = dh2.n("rightHigh");
    }

    @Override
    public void a(avk avk2, List list, Random random) {
        int n2 = 3;
        int n3 = 5;
        if (this.g == 1 || this.g == 2) {
            n2 = 8 - n2;
            n3 = 8 - n3;
        }
        this.a((auz)avk2, list, random, 5, 1);
        if (this.a) {
            this.b((auz)avk2, list, random, n2, 1);
        }
        if (this.b) {
            this.b((auz)avk2, list, random, n3, 7);
        }
        if (this.c) {
            this.c((auz)avk2, list, random, n2, 1);
        }
        if (this.e) {
            this.c((auz)avk2, list, random, n3, 7);
        }
    }

    public static auo a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        asv asv2 = asv.a(n2, n3, n4, -4, -3, 0, 10, 9, 11, n5);
        if (!auo.a(asv2) || avk.a(list, asv2) != null) {
            return null;
        }
        return new auo(n6, random, asv2, n5);
    }

    @Override
    public boolean a(ahb ahb2, Random random, asv asv2) {
        if (this.a(ahb2, asv2)) {
            return false;
        }
        this.a(ahb2, asv2, 0, 0, 0, 9, 8, 10, true, random, aui.c());
        this.a(ahb2, random, asv2, this.d, 4, 3, 0);
        if (this.a) {
            this.a(ahb2, asv2, 0, 3, 1, 0, 5, 3, ajn.a, ajn.a, false);
        }
        if (this.c) {
            this.a(ahb2, asv2, 9, 3, 1, 9, 5, 3, ajn.a, ajn.a, false);
        }
        if (this.b) {
            this.a(ahb2, asv2, 0, 5, 7, 0, 7, 9, ajn.a, ajn.a, false);
        }
        if (this.e) {
            this.a(ahb2, asv2, 9, 5, 7, 9, 7, 9, ajn.a, ajn.a, false);
        }
        this.a(ahb2, asv2, 5, 1, 10, 7, 3, 10, ajn.a, ajn.a, false);
        this.a(ahb2, asv2, 1, 2, 1, 8, 2, 6, false, random, aui.c());
        this.a(ahb2, asv2, 4, 1, 5, 4, 4, 9, false, random, aui.c());
        this.a(ahb2, asv2, 8, 1, 5, 8, 4, 9, false, random, aui.c());
        this.a(ahb2, asv2, 1, 4, 7, 3, 4, 9, false, random, aui.c());
        this.a(ahb2, asv2, 1, 3, 5, 3, 3, 6, false, random, aui.c());
        this.a(ahb2, asv2, 1, 3, 4, 3, 3, 4, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 1, 4, 6, 3, 4, 6, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 5, 1, 7, 7, 1, 8, false, random, aui.c());
        this.a(ahb2, asv2, 5, 1, 9, 7, 1, 9, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 5, 2, 7, 7, 2, 7, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 4, 5, 7, 4, 5, 9, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 8, 5, 7, 8, 5, 9, ajn.U, ajn.U, false);
        this.a(ahb2, asv2, 5, 5, 7, 7, 5, 9, ajn.T, ajn.T, false);
        this.a(ahb2, ajn.aa, 0, 6, 5, 6, asv2);
        return true;
    }
}

