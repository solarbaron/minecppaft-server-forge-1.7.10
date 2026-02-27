/*
 * Decompiled with CFR 0.152.
 */
public class sq
extends sa {
    public int a;
    public int b;
    public int c;
    private int d = 5;
    private int e;
    private yz f;
    private int g;

    public sq(ahb ahb2, double d2, double d3, double d4, int n2) {
        super(ahb2);
        this.a(0.5f, 0.5f);
        this.L = this.N / 2.0f;
        this.b(d2, d3, d4);
        this.y = (float)(Math.random() * 360.0);
        this.v = (float)(Math.random() * (double)0.2f - (double)0.1f) * 2.0f;
        this.w = (float)(Math.random() * 0.2) * 2.0f;
        this.x = (float)(Math.random() * (double)0.2f - (double)0.1f) * 2.0f;
        this.e = n2;
    }

    @Override
    protected boolean g_() {
        return false;
    }

    public sq(ahb ahb2) {
        super(ahb2);
        this.a(0.25f, 0.25f);
        this.L = this.N / 2.0f;
    }

    @Override
    protected void c() {
    }

    @Override
    public void h() {
        double d2;
        double d3;
        double d4;
        double d5;
        double d6;
        super.h();
        if (this.c > 0) {
            --this.c;
        }
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        this.w -= (double)0.03f;
        if (this.o.a(qh.c(this.s), qh.c(this.t), qh.c(this.u)).o() == awt.i) {
            this.w = 0.2f;
            this.v = (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f;
            this.x = (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f;
            this.a("random.fizz", 0.4f, 2.0f + this.Z.nextFloat() * 0.4f);
        }
        this.j(this.s, (this.C.b + this.C.e) / 2.0, this.u);
        double d7 = 8.0;
        if (this.g < this.a - 20 + this.y() % 100) {
            if (this.f == null || this.f.f(this) > d7 * d7) {
                this.f = this.o.a((sa)this, d7);
            }
            this.g = this.a;
        }
        if (this.f != null && (d6 = 1.0 - (d5 = Math.sqrt((d4 = (this.f.s - this.s) / d7) * d4 + (d3 = (this.f.t + (double)this.f.g() - this.t) / d7) * d3 + (d2 = (this.f.u - this.u) / d7) * d2))) > 0.0) {
            d6 *= d6;
            this.v += d4 / d5 * d6 * 0.1;
            this.w += d3 / d5 * d6 * 0.1;
            this.x += d2 / d5 * d6 * 0.1;
        }
        this.d(this.v, this.w, this.x);
        float f2 = 0.98f;
        if (this.D) {
            f2 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.98f;
        }
        this.v *= (double)f2;
        this.w *= (double)0.98f;
        this.x *= (double)f2;
        if (this.D) {
            this.w *= (double)-0.9f;
        }
        ++this.a;
        ++this.b;
        if (this.b >= 6000) {
            this.B();
        }
    }

    @Override
    public boolean N() {
        return this.o.a(this.C, awt.h, (sa)this);
    }

    @Override
    protected void f(int n2) {
        this.a(ro.a, (float)n2);
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        this.Q();
        this.d = (int)((float)this.d - f2);
        if (this.d <= 0) {
            this.B();
        }
        return false;
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Health", (short)((byte)this.d));
        dh2.a("Age", (short)this.b);
        dh2.a("Value", (short)this.e);
    }

    @Override
    public void a(dh dh2) {
        this.d = dh2.e("Health") & 0xFF;
        this.b = dh2.e("Age");
        this.e = dh2.e("Value");
    }

    @Override
    public void b_(yz yz2) {
        if (this.o.E) {
            return;
        }
        if (this.c == 0 && yz2.bt == 0) {
            yz2.bt = 2;
            this.o.a((sa)yz2, "random.orb", 0.1f, 0.5f * ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.7f + 1.8f));
            yz2.a((sa)this, 1);
            yz2.v(this.e);
            this.B();
        }
    }

    public int e() {
        return this.e;
    }

    public static int a(int n2) {
        if (n2 >= 2477) {
            return 2477;
        }
        if (n2 >= 1237) {
            return 1237;
        }
        if (n2 >= 617) {
            return 617;
        }
        if (n2 >= 307) {
            return 307;
        }
        if (n2 >= 149) {
            return 149;
        }
        if (n2 >= 73) {
            return 73;
        }
        if (n2 >= 37) {
            return 37;
        }
        if (n2 >= 17) {
            return 17;
        }
        if (n2 >= 7) {
            return 7;
        }
        if (n2 >= 3) {
            return 3;
        }
        return 1;
    }

    @Override
    public boolean av() {
        return false;
    }
}

