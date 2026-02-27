/*
 * Decompiled with CFR 0.152.
 */
public class cm
implements cp {
    @Override
    public final add a(ck ck2, add add2) {
        add add3 = this.b(ck2, add2);
        this.a(ck2);
        this.a(ck2, akm.b(ck2.h()));
        return add3;
    }

    protected add b(ck ck2, add add2) {
        cr cr2 = akm.b(ck2.h());
        cx cx2 = akm.a(ck2);
        add add3 = add2.a(1);
        cm.a(ck2.k(), add3, 6, cr2, cx2);
        return add2;
    }

    public static void a(ahb ahb2, add add2, int n2, cr cr2, cx cx2) {
        double d2 = cx2.a();
        double d3 = cx2.b();
        double d4 = cx2.c();
        xk xk2 = new xk(ahb2, d2, d3 - 0.3, d4, add2);
        double d5 = ahb2.s.nextDouble() * 0.1 + 0.2;
        xk2.v = (double)cr2.c() * d5;
        xk2.w = 0.2f;
        xk2.x = (double)cr2.e() * d5;
        xk2.v += ahb2.s.nextGaussian() * (double)0.0075f * (double)n2;
        xk2.w += ahb2.s.nextGaussian() * (double)0.0075f * (double)n2;
        xk2.x += ahb2.s.nextGaussian() * (double)0.0075f * (double)n2;
        ahb2.d(xk2);
    }

    protected void a(ck ck2) {
        ck2.k().c(1000, ck2.d(), ck2.e(), ck2.f(), 0);
    }

    protected void a(ck ck2, cr cr2) {
        ck2.k().c(2000, ck2.d(), ck2.e(), ck2.f(), this.a(cr2));
    }

    private int a(cr cr2) {
        return cr2.c() + 1 + (cr2.e() + 1) * 3;
    }
}

