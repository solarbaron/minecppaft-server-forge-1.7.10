/*
 * Decompiled with CFR 0.152.
 */
public abstract class uc
extends ui {
    protected sw a;
    protected int b;
    protected int c;
    protected int d;
    protected akn e;
    boolean f;
    float g;
    float h;

    public uc(sw sw2) {
        this.a = sw2;
    }

    @Override
    public boolean a() {
        if (!this.a.E) {
            return false;
        }
        vv vv2 = this.a.m();
        ayf ayf2 = vv2.e();
        if (ayf2 == null || ayf2.b() || !vv2.c()) {
            return false;
        }
        for (int i2 = 0; i2 < Math.min(ayf2.e() + 2, ayf2.d()); ++i2) {
            aye aye2 = ayf2.a(i2);
            this.b = aye2.a;
            this.c = aye2.b + 1;
            this.d = aye2.c;
            if (this.a.e(this.b, this.a.t, this.d) > 2.25) continue;
            this.e = this.a(this.b, this.c, this.d);
            if (this.e == null) continue;
            return true;
        }
        this.b = qh.c(this.a.s);
        this.c = qh.c(this.a.t + 1.0);
        this.d = qh.c(this.a.u);
        this.e = this.a(this.b, this.c, this.d);
        return this.e != null;
    }

    @Override
    public boolean b() {
        return !this.f;
    }

    @Override
    public void c() {
        this.f = false;
        this.g = (float)((double)((float)this.b + 0.5f) - this.a.s);
        this.h = (float)((double)((float)this.d + 0.5f) - this.a.u);
    }

    @Override
    public void e() {
        float f2 = (float)((double)((float)this.b + 0.5f) - this.a.s);
        float f3 = (float)((double)((float)this.d + 0.5f) - this.a.u);
        float f4 = this.g * f2 + this.h * f3;
        if (f4 < 0.0f) {
            this.f = true;
        }
    }

    private akn a(int n2, int n3, int n4) {
        aji aji2 = this.a.o.a(n2, n3, n4);
        if (aji2 != ajn.ao) {
            return null;
        }
        return (akn)aji2;
    }
}

