/*
 * Decompiled with CFR 0.152.
 */
public class alq
extends ajc {
    protected alq() {
        super(awt.d);
        this.a(abt.c);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.e(n2, n3, n4) == 0) {
            return false;
        }
        this.e(ahb2, n2, n3, n4);
        return true;
    }

    public void b(ahb ahb2, int n2, int n3, int n4, add add2) {
        if (ahb2.E) {
            return;
        }
        alr alr2 = (alr)ahb2.o(n2, n3, n4);
        if (alr2 == null) {
            return;
        }
        alr2.a(add2.m());
        ahb2.a(n2, n3, n4, 1, 2);
    }

    public void e(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.E) {
            return;
        }
        alr alr2 = (alr)ahb2.o(n2, n3, n4);
        if (alr2 == null) {
            return;
        }
        add add2 = alr2.a();
        if (add2 == null) {
            return;
        }
        ahb2.c(1005, n2, n3, n4, 0);
        ahb2.a((String)null, n2, n3, n4);
        alr2.a((add)null);
        ahb2.a(n2, n3, n4, 0, 2);
        float f2 = 0.7f;
        double d2 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.5;
        double d3 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.2 + 0.6;
        double d4 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.5;
        add add3 = add2.m();
        xk xk2 = new xk(ahb2, (double)n2 + d2, (double)n3 + d3, (double)n4 + d4, add3);
        xk2.b = 10;
        ahb2.d(xk2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        this.e(ahb2, n2, n3, n4);
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (ahb2.E) {
            return;
        }
        super.a(ahb2, n2, n3, n4, n5, f2, 0);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new alr();
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        add add2 = ((alr)ahb2.o(n2, n3, n4)).a();
        return add2 == null ? 0 : adb.b(add2.b()) + 1 - adb.b(ade.cd);
    }
}

