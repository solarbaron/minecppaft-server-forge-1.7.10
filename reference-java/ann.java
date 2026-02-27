/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ann
extends aji {
    protected ann() {
        super(awt.y);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
        this.a(true);
        this.a(abt.c);
        this.b(0);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4) & 7;
        float f2 = 0.125f;
        return azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (float)n3 + (float)n5 * f2, (double)n4 + this.G);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public void g() {
        this.b(0);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.b(ahl2.e(n2, n3, n4));
    }

    protected void b(int n2) {
        int n3 = n2 & 7;
        float f2 = (float)(2 * (1 + n3)) / 16.0f;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, f2, 1.0f);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3 - 1, n4);
        if (aji2 == ajn.aD || aji2 == ajn.cj) {
            return false;
        }
        if (aji2.o() == awt.j) {
            return true;
        }
        if (aji2 == this && (ahb2.e(n2, n3 - 1, n4) & 7) == 7) {
            return true;
        }
        return aji2.c() && aji2.J.c();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.m(ahb2, n2, n3, n4);
    }

    private boolean m(ahb ahb2, int n2, int n3, int n4) {
        if (!this.c(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            return false;
        }
        return true;
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        int n6 = n5 & 7;
        this.a(ahb2, n2, n3, n4, new add(ade.ay, n6 + 1, 0));
        ahb2.f(n2, n3, n4);
        yz2.a(pp.C[aji.b(this)], 1);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ay;
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.b(ahn.b, n2, n3, n4) > 11) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
        }
    }
}

