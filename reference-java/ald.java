/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ald
extends ajc {
    public ald() {
        super(awt.q);
        this.g();
    }

    @Override
    public void g() {
        float f2 = 0.375f;
        float f3 = f2 / 2.0f;
        this.a(0.5f - f3, 0.0f, 0.5f - f3, 0.5f + f3, f2, 0.5f + f3);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public int b() {
        return 33;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        add add2 = yz2.bm.h();
        if (add2 == null || !(add2.b() instanceof abh)) {
            return false;
        }
        apf apf2 = this.e(ahb2, n2, n3, n4);
        if (apf2 != null) {
            if (apf2.a() != null) {
                return false;
            }
            aji aji2 = aji.a(add2.b());
            if (!this.a(aji2, add2.k())) {
                return false;
            }
            apf2.a(add2.b(), add2.k());
            apf2.e();
            if (!ahb2.a(n2, n3, n4, add2.k(), 2)) {
                ahb2.g(n2, n3, n4);
            }
            if (!yz2.bE.d && --add2.b <= 0) {
                yz2.bm.a(yz2.bm.c, null);
            }
            return true;
        }
        return false;
    }

    private boolean a(aji aji2, int n2) {
        if (aji2 == ajn.N || aji2 == ajn.O || aji2 == ajn.aF || aji2 == ajn.P || aji2 == ajn.Q || aji2 == ajn.g || aji2 == ajn.I) {
            return true;
        }
        return aji2 == ajn.H && n2 == 2;
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        apf apf2 = this.e(ahb2, n2, n3, n4);
        if (apf2 != null && apf2.a() != null) {
            return apf2.b();
        }
        return 0;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return super.c(ahb2, n2, n3, n4) && ahb.a(ahb2, n2, n3 - 1, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        apf apf2 = this.e(ahb2, n2, n3, n4);
        if (apf2 != null && apf2.a() != null) {
            this.a(ahb2, n2, n3, n4, new add(apf2.a(), 1, apf2.b()));
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        apf apf2;
        super.a(ahb2, n2, n3, n4, n5, yz2);
        if (yz2.bE.d && (apf2 = this.e(ahb2, n2, n3, n4)) != null) {
            apf2.a(adb.d(0), 0);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.bE;
    }

    private apf e(ahb ahb2, int n2, int n3, int n4) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null && aor2 instanceof apf) {
            return (apf)aor2;
        }
        return null;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        aji aji2 = null;
        int n3 = 0;
        switch (n2) {
            default: {
                break;
            }
            case 1: {
                aji2 = ajn.O;
                n3 = 0;
                break;
            }
            case 2: {
                aji2 = ajn.N;
                break;
            }
            case 3: {
                aji2 = ajn.g;
                n3 = 0;
                break;
            }
            case 4: {
                aji2 = ajn.g;
                n3 = 1;
                break;
            }
            case 5: {
                aji2 = ajn.g;
                n3 = 2;
                break;
            }
            case 6: {
                aji2 = ajn.g;
                n3 = 3;
                break;
            }
            case 12: {
                aji2 = ajn.g;
                n3 = 4;
                break;
            }
            case 13: {
                aji2 = ajn.g;
                n3 = 5;
                break;
            }
            case 7: {
                aji2 = ajn.Q;
                break;
            }
            case 8: {
                aji2 = ajn.P;
                break;
            }
            case 9: {
                aji2 = ajn.aF;
                break;
            }
            case 10: {
                aji2 = ajn.I;
                break;
            }
            case 11: {
                aji2 = ajn.H;
                n3 = 2;
            }
        }
        return new apf(adb.a(aji2), n3);
    }
}

