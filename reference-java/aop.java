/*
 * Decompiled with CFR 0.152.
 */
public class aop
extends aji {
    protected aop() {
        super(awt.r);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.0625f, 1.0f);
        this.a(true);
        this.a(abt.c);
        this.b(0);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        boolean bl2 = false;
        float f2 = 0.0625f;
        return azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (float)n3 + (float)bl2 * f2, (double)n4 + this.G);
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
        int n3 = 0;
        float f2 = (float)(1 * (1 + n3)) / 16.0f;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, f2, 1.0f);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return super.c(ahb2, n2, n3, n4) && this.j(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.e(ahb2, n2, n3, n4);
    }

    private boolean e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            return false;
        }
        return true;
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return !ahb2.c(n2, n3 - 1, n4);
    }

    @Override
    public int a(int n2) {
        return n2;
    }
}

