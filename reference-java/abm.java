/*
 * Decompiled with CFR 0.152.
 */
public class abm
extends adb {
    public static final String[] a = new String[]{"pulling_0", "pulling_1", "pulling_2"};

    public abm() {
        this.h = 1;
        this.f(384);
        this.a(abt.j);
    }

    @Override
    public void a(add add2, ahb ahb2, yz yz2, int n2) {
        boolean bl2;
        boolean bl3 = bl2 = yz2.bE.d || afv.a(aft.y.B, add2) > 0;
        if (bl2 || yz2.bm.b(ade.g)) {
            int n3;
            int n4;
            int n5 = this.d_(add2) - n2;
            float f2 = (float)n5 / 20.0f;
            if ((double)(f2 = (f2 * f2 + f2 * 2.0f) / 3.0f) < 0.1) {
                return;
            }
            if (f2 > 1.0f) {
                f2 = 1.0f;
            }
            zc zc2 = new zc(ahb2, yz2, f2 * 2.0f);
            if (f2 == 1.0f) {
                zc2.a(true);
            }
            if ((n4 = afv.a(aft.v.B, add2)) > 0) {
                zc2.b(zc2.e() + (double)n4 * 0.5 + 0.5);
            }
            if ((n3 = afv.a(aft.w.B, add2)) > 0) {
                zc2.a(n3);
            }
            if (afv.a(aft.x.B, add2) > 0) {
                zc2.e(100);
            }
            add2.a(1, (sv)yz2);
            ahb2.a((sa)yz2, "random.bow", 1.0f, 1.0f / (g.nextFloat() * 0.4f + 1.2f) + f2 * 0.5f);
            if (bl2) {
                zc2.a = 2;
            } else {
                yz2.bm.a(ade.g);
            }
            if (!ahb2.E) {
                ahb2.d(zc2);
            }
        }
    }

    @Override
    public add b(add add2, ahb ahb2, yz yz2) {
        return add2;
    }

    @Override
    public int d_(add add2) {
        return 72000;
    }

    @Override
    public aei d(add add2) {
        return aei.e;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (yz2.bE.d || yz2.bm.b(ade.g)) {
            yz2.a(add2, this.d_(add2));
        }
        return add2;
    }

    @Override
    public int c() {
        return 1;
    }
}

