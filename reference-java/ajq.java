/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ajq
extends ajc {
    private Random a = new Random();

    public ajq() {
        super(awt.f);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public int b() {
        return 25;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new aov();
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a(0.4375f, 0.0f, 0.4375f, 0.5625f, 0.875f, 0.5625f);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        this.g();
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
    }

    @Override
    public void g() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        aov aov2 = (aov)ahb2.o(n2, n3, n4);
        if (aov2 != null) {
            yz2.a(aov2);
        }
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        if (add2.u()) {
            ((aov)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 instanceof aov) {
            aov aov2 = (aov)aor2;
            for (int i2 = 0; i2 < aov2.a(); ++i2) {
                add add2 = aov2.a(i2);
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
                    float f5 = 0.05f;
                    xk2.v = (float)this.a.nextGaussian() * f5;
                    xk2.w = (float)this.a.nextGaussian() * f5 + 0.2f;
                    xk2.x = (float)this.a.nextGaussian() * f5;
                    ahb2.d(xk2);
                }
            }
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.bt;
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

