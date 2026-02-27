/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aju
extends aji {
    protected aju() {
        super(awt.F);
        this.a(true);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        float f2 = 0.0625f;
        float f3 = (float)(1 + n5 * 2) / 16.0f;
        float f4 = 0.5f;
        this.a(f3, 0.0f, f2, 1.0f - f2, f4, 1.0f - f2);
    }

    @Override
    public void g() {
        float f2 = 0.0625f;
        float f3 = 0.5f;
        this.a(f2, 0.0f, f2, 1.0f - f2, f3, 1.0f - f2);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        float f2 = 0.0625f;
        float f3 = (float)(1 + n5 * 2) / 16.0f;
        float f4 = 0.5f;
        return azt.a((float)n2 + f3, n3, (float)n4 + f2, (float)(n2 + 1) - f2, (float)n3 + f4 - f2, (float)(n4 + 1) - f2);
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        this.b(ahb2, n2, n3, n4, yz2);
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        this.b(ahb2, n2, n3, n4, yz2);
    }

    private void b(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        if (yz2.g(false)) {
            yz2.bQ().a(2, 0.1f);
            int n5 = ahb2.e(n2, n3, n4) + 1;
            if (n5 >= 6) {
                ahb2.f(n2, n3, n4);
            } else {
                ahb2.a(n2, n3, n4, n5, 2);
            }
        }
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (!super.c(ahb2, n2, n3, n4)) {
            return false;
        }
        return this.j(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!this.j(ahb2, n2, n3, n4)) {
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.a(n2, n3 - 1, n4).o().a();
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }
}

