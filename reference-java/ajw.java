/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ajw
extends aji {
    public ajw() {
        super(awt.f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.3125f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        float f2 = 0.125f;
        this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        this.g();
    }

    @Override
    public void g() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public int b() {
        return 24;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        int n5 = ajw.b(ahb2.e(n2, n3, n4));
        float f2 = (float)n3 + (6.0f + (float)(3 * n5)) / 16.0f;
        if (!ahb2.E && sa2.al() && n5 > 0 && sa2.C.b <= (double)f2) {
            sa2.F();
            this.a(ahb2, n2, n3, n4, n5 - 1);
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        add add2 = yz2.bm.h();
        if (add2 == null) {
            return true;
        }
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = ajw.b(n6);
        if (add2.b() == ade.as) {
            if (n7 < 3) {
                if (!yz2.bE.d) {
                    yz2.bm.a(yz2.bm.c, new add(ade.ar));
                }
                this.a(ahb2, n2, n3, n4, 3);
            }
            return true;
        }
        if (add2.b() == ade.bo) {
            if (n7 > 0) {
                if (!yz2.bE.d) {
                    add add3 = new add(ade.bn, 1, 0);
                    if (!yz2.bm.a(add3)) {
                        ahb2.d(new xk(ahb2, (double)n2 + 0.5, (double)n3 + 1.5, (double)n4 + 0.5, add3));
                    } else if (yz2 instanceof mw) {
                        ((mw)yz2).a(yz2.bn);
                    }
                    --add2.b;
                    if (add2.b <= 0) {
                        yz2.bm.a(yz2.bm.c, null);
                    }
                }
                this.a(ahb2, n2, n3, n4, n7 - 1);
            }
        } else if (n7 > 0 && add2.b() instanceof abb && ((abb)add2.b()).m_() == abd.a) {
            abb abb2 = (abb)add2.b();
            abb2.c(add2);
            this.a(ahb2, n2, n3, n4, n7 - 1);
            return true;
        }
        return false;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        ahb2.a(n2, n3, n4, qh.a(n5, 0, 3), 2);
        ahb2.f(n2, n3, n4, this);
    }

    @Override
    public void l(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.s.nextInt(20) != 1) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if (n5 < 3) {
            ahb2.a(n2, n3, n4, n5 + 1, 2);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.bu;
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = ahb2.e(n2, n3, n4);
        return ajw.b(n6);
    }

    public static int b(int n2) {
        return n2;
    }
}

