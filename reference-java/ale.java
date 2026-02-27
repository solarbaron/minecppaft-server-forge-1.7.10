/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ale
extends ajc {
    private final Random a = new Random();
    private final boolean b;
    private static boolean M;

    protected ale(boolean bl2) {
        super(awt.e);
        this.b = bl2;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.al);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        this.e(ahb2, n2, n3, n4);
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
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
        apg apg2 = (apg)ahb2.o(n2, n3, n4);
        if (apg2 != null) {
            yz2.a(apg2);
        }
        return true;
    }

    public static void a(boolean bl2, ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        aor aor2 = ahb2.o(n2, n3, n4);
        M = true;
        if (bl2) {
            ahb2.b(n2, n3, n4, ajn.am);
        } else {
            ahb2.b(n2, n3, n4, ajn.al);
        }
        M = false;
        ahb2.a(n2, n3, n4, n5, 2);
        if (aor2 != null) {
            aor2.t();
            ahb2.a(n2, n3, n4, aor2);
        }
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apg();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3;
        if (n5 == 0) {
            ahb2.a(n2, n3, n4, 2, 2);
        }
        if (n5 == 1) {
            ahb2.a(n2, n3, n4, 5, 2);
        }
        if (n5 == 2) {
            ahb2.a(n2, n3, n4, 3, 2);
        }
        if (n5 == 3) {
            ahb2.a(n2, n3, n4, 4, 2);
        }
        if (add2.u()) {
            ((apg)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        apg apg2;
        if (!M && (apg2 = (apg)ahb2.o(n2, n3, n4)) != null) {
            for (int i2 = 0; i2 < apg2.a(); ++i2) {
                add add2 = apg2.a(i2);
                if (add2 == null) continue;
                float f2 = this.a.nextFloat() * 0.8f + 0.1f;
                float f3 = this.a.nextFloat() * 0.8f + 0.1f;
                float f4 = this.a.nextFloat() * 0.8f + 0.1f;
                while (add2.b > 0) {
                    int n6 = this.a.nextInt(21) + 10;
                    if (n6 > add2.b) {
                        n6 = add2.b;
                    }
                    add2.b -= n6;
                    xk xk2 = new xk(ahb2, (float)n2 + f2, (float)n3 + f3, (float)n4 + f4, new add(add2.b(), n6, add2.k()));
                    if (add2.p()) {
                        xk2.f().d((dh)add2.q().b());
                    }
                    float f5 = 0.05f;
                    xk2.v = (float)this.a.nextGaussian() * f5;
                    xk2.w = (float)this.a.nextGaussian() * f5 + 0.2f;
                    xk2.x = (float)this.a.nextGaussian() * f5;
                    ahb2.d(xk2);
                }
            }
            ahb2.f(n2, n3, n4, aji2);
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
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

