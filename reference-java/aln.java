/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aln
extends ajc {
    private final Random a = new Random();

    public aln() {
        super(awt.f);
        this.a(abt.d);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.625f, 1.0f);
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
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = q.a[n5];
        if (n7 == 1) {
            n7 = 0;
        }
        return n7;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new api();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        super.a(ahb2, n2, n3, n4, sv2, add2);
        if (add2.u()) {
            api api2 = aln.e(ahb2, n2, n3, n4);
            api2.a(add2.s());
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        this.e(ahb2, n2, n3, n4);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        api api2 = aln.e(ahb2, n2, n3, n4);
        if (api2 != null) {
            yz2.a(api2);
        }
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.e(ahb2, n2, n3, n4);
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        boolean bl2;
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = aln.b(n5);
        boolean bl3 = !ahb2.v(n2, n3, n4);
        if (bl3 != (bl2 = aln.c(n5))) {
            ahb2.a(n2, n3, n4, n6 | (bl3 ? 0 : 8), 4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        api api2 = (api)ahb2.o(n2, n3, n4);
        if (api2 != null) {
            for (int i2 = 0; i2 < api2.a(); ++i2) {
                add add2 = api2.a(i2);
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
    public int b() {
        return 38;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    public static int b(int n2) {
        return n2 & 7;
    }

    public static boolean c(int n2) {
        return (n2 & 8) != 8;
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        return zs.b(aln.e(ahb2, n2, n3, n4));
    }

    public static api e(ahl ahl2, int n2, int n3, int n4) {
        return (api)ahl2.o(n2, n3, n4);
    }
}

