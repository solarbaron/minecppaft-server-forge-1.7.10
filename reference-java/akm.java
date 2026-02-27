/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akm
extends ajc {
    public static final cz a = new co(new cm());
    protected Random b = new Random();

    protected akm() {
        super(awt.e);
        this.a(abt.d);
    }

    @Override
    public int a(ahb ahb2) {
        return 4;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        this.m(ahb2, n2, n3, n4);
    }

    private void m(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.E) {
            return;
        }
        aji aji2 = ahb2.a(n2, n3, n4 - 1);
        aji aji3 = ahb2.a(n2, n3, n4 + 1);
        aji aji4 = ahb2.a(n2 - 1, n3, n4);
        aji aji5 = ahb2.a(n2 + 1, n3, n4);
        int n5 = 3;
        if (aji2.j() && !aji3.j()) {
            n5 = 3;
        }
        if (aji3.j() && !aji2.j()) {
            n5 = 2;
        }
        if (aji4.j() && !aji5.j()) {
            n5 = 5;
        }
        if (aji5.j() && !aji4.j()) {
            n5 = 4;
        }
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        apb apb2 = (apb)ahb2.o(n2, n3, n4);
        if (apb2 != null) {
            yz2.a(apb2);
        }
        return true;
    }

    protected void e(ahb ahb2, int n2, int n3, int n4) {
        cl cl2 = new cl(ahb2, n2, n3, n4);
        apb apb2 = (apb)cl2.j();
        if (apb2 == null) {
            return;
        }
        int n5 = apb2.i();
        if (n5 < 0) {
            ahb2.c(1001, n2, n3, n4, 0);
        } else {
            add add2 = apb2.a(n5);
            cp cp2 = this.a(add2);
            if (cp2 != cp.a) {
                add add3 = cp2.a(cl2, add2);
                apb2.a(n5, add3.b == 0 ? null : add3);
            }
        }
    }

    protected cp a(add add2) {
        return (cp)a.a(add2.b());
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        boolean bl2;
        boolean bl3 = ahb2.v(n2, n3, n4) || ahb2.v(n2, n3 + 1, n4);
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl4 = bl2 = (n5 & 8) != 0;
        if (bl3 && !bl2) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
            ahb2.a(n2, n3, n4, n5 | 8, 4);
        } else if (!bl3 && bl2) {
            ahb2.a(n2, n3, n4, n5 & 0xFFFFFFF7, 4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (!ahb2.E) {
            this.e(ahb2, n2, n3, n4);
        }
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apb();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = app.a(ahb2, n2, n3, n4, sv2);
        ahb2.a(n2, n3, n4, n5, 2);
        if (add2.u()) {
            ((apb)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        apb apb2 = (apb)ahb2.o(n2, n3, n4);
        if (apb2 != null) {
            for (int i2 = 0; i2 < apb2.a(); ++i2) {
                add add2 = apb2.a(i2);
                if (add2 == null) continue;
                float f2 = this.b.nextFloat() * 0.8f + 0.1f;
                float f3 = this.b.nextFloat() * 0.8f + 0.1f;
                float f4 = this.b.nextFloat() * 0.8f + 0.1f;
                while (add2.b > 0) {
                    int n6 = this.b.nextInt(21) + 10;
                    if (n6 > add2.b) {
                        n6 = add2.b;
                    }
                    add2.b -= n6;
                    xk xk2 = new xk(ahb2, (float)n2 + f2, (float)n3 + f3, (float)n4 + f4, new add(add2.b(), n6, add2.k()));
                    if (add2.p()) {
                        xk2.f().d((dh)add2.q().b());
                    }
                    float f5 = 0.05f;
                    xk2.v = (float)this.b.nextGaussian() * f5;
                    xk2.w = (float)this.b.nextGaussian() * f5 + 0.2f;
                    xk2.x = (float)this.b.nextGaussian() * f5;
                    ahb2.d(xk2);
                }
            }
            ahb2.f(n2, n3, n4, aji2);
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    public static cx a(ck ck2) {
        cr cr2 = akm.b(ck2.h());
        double d2 = ck2.a() + 0.7 * (double)cr2.c();
        double d3 = ck2.b() + 0.7 * (double)cr2.d();
        double d4 = ck2.c() + 0.7 * (double)cr2.e();
        return new cy(d2, d3, d4);
    }

    public static cr b(int n2) {
        return cr.a(n2 & 7);
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        return zs.b((rb)((Object)ahb2.o(n2, n3, n4)));
    }
}

