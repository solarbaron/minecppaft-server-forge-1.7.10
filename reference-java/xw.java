/*
 * Decompiled with CFR 0.152.
 */
public class xw
extends sa {
    public int a;
    private sv b;

    public xw(ahb ahb2) {
        super(ahb2);
        this.k = true;
        this.a(0.98f, 0.98f);
        this.L = this.N / 2.0f;
    }

    public xw(ahb ahb2, double d2, double d3, double d4, sv sv2) {
        this(ahb2);
        this.b(d2, d3, d4);
        float f2 = (float)(Math.random() * 3.1415927410125732 * 2.0);
        this.v = -((float)Math.sin(f2)) * 0.02f;
        this.w = 0.2f;
        this.x = -((float)Math.cos(f2)) * 0.02f;
        this.a = 80;
        this.p = d2;
        this.q = d3;
        this.r = d4;
        this.b = sv2;
    }

    @Override
    protected void c() {
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    public boolean R() {
        return !this.K;
    }

    @Override
    public void h() {
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        this.w -= (double)0.04f;
        this.d(this.v, this.w, this.x);
        this.v *= (double)0.98f;
        this.w *= (double)0.98f;
        this.x *= (double)0.98f;
        if (this.D) {
            this.v *= (double)0.7f;
            this.x *= (double)0.7f;
            this.w *= -0.5;
        }
        if (this.a-- <= 0) {
            this.B();
            if (!this.o.E) {
                this.f();
            }
        } else {
            this.o.a("smoke", this.s, this.t + 0.5, this.u, 0.0, 0.0, 0.0);
        }
    }

    private void f() {
        float f2 = 4.0f;
        this.o.a(this, this.s, this.t, this.u, f2, true);
    }

    @Override
    protected void b(dh dh2) {
        dh2.a("Fuse", (byte)this.a);
    }

    @Override
    protected void a(dh dh2) {
        this.a = dh2.d("Fuse");
    }

    public sv e() {
        return this.b;
    }
}

