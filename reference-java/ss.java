/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public abstract class ss
extends sa {
    private int e;
    public int a;
    public int b;
    public int c;
    public int d;

    public ss(ahb ahb2) {
        super(ahb2);
        this.L = 0.0f;
        this.a(0.5f, 0.5f);
    }

    public ss(ahb ahb2, int n2, int n3, int n4, int n5) {
        this(ahb2);
        this.b = n2;
        this.c = n3;
        this.d = n4;
    }

    @Override
    protected void c() {
    }

    public void a(int n2) {
        this.a = n2;
        this.A = this.y = (float)(n2 * 90);
        float f2 = this.f();
        float f3 = this.i();
        float f4 = this.f();
        if (n2 == 2 || n2 == 0) {
            f4 = 0.5f;
            this.y = this.A = (float)(p.f[n2] * 90);
        } else {
            f2 = 0.5f;
        }
        f2 /= 32.0f;
        f3 /= 32.0f;
        f4 /= 32.0f;
        float f5 = (float)this.b + 0.5f;
        float f6 = (float)this.c + 0.5f;
        float f7 = (float)this.d + 0.5f;
        float f8 = 0.5625f;
        if (n2 == 2) {
            f7 -= f8;
        }
        if (n2 == 1) {
            f5 -= f8;
        }
        if (n2 == 0) {
            f7 += f8;
        }
        if (n2 == 3) {
            f5 += f8;
        }
        if (n2 == 2) {
            f5 -= this.c(this.f());
        }
        if (n2 == 1) {
            f7 += this.c(this.f());
        }
        if (n2 == 0) {
            f5 += this.c(this.f());
        }
        if (n2 == 3) {
            f7 -= this.c(this.f());
        }
        this.b(f5, f6 += this.c(this.i()), f7);
        float f9 = -0.03125f;
        this.C.b(f5 - f2 - f9, f6 - f3 - f9, f7 - f4 - f9, f5 + f2 + f9, f6 + f3 + f9, f7 + f4 + f9);
    }

    private float c(int n2) {
        if (n2 == 32) {
            return 0.5f;
        }
        if (n2 == 64) {
            return 0.5f;
        }
        return 0.0f;
    }

    @Override
    public void h() {
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        if (this.e++ == 100 && !this.o.E) {
            this.e = 0;
            if (!this.K && !this.e()) {
                this.B();
                this.b((sa)null);
            }
        }
    }

    public boolean e() {
        if (!this.o.a((sa)this, this.C).isEmpty()) {
            return false;
        }
        int n2 = Math.max(1, this.f() / 16);
        int n3 = Math.max(1, this.i() / 16);
        int n4 = this.b;
        int n5 = this.c;
        int n6 = this.d;
        if (this.a == 2) {
            n4 = qh.c(this.s - (double)((float)this.f() / 32.0f));
        }
        if (this.a == 1) {
            n6 = qh.c(this.u - (double)((float)this.f() / 32.0f));
        }
        if (this.a == 0) {
            n4 = qh.c(this.s - (double)((float)this.f() / 32.0f));
        }
        if (this.a == 3) {
            n6 = qh.c(this.u - (double)((float)this.f() / 32.0f));
        }
        n5 = qh.c(this.t - (double)((float)this.i() / 32.0f));
        for (int i2 = 0; i2 < n2; ++i2) {
            for (int i3 = 0; i3 < n3; ++i3) {
                Object object = this.a == 2 || this.a == 0 ? this.o.a(n4 + i2, n5 + i3, this.d).o() : this.o.a(this.b, n5 + i3, n6 + i2).o();
                if (((awt)object).a()) continue;
                return false;
            }
        }
        List list = this.o.b((sa)this, this.C);
        for (Object object : list) {
            if (!(object instanceof ss)) continue;
            return false;
        }
        return true;
    }

    @Override
    public boolean R() {
        return true;
    }

    @Override
    public boolean j(sa sa2) {
        if (sa2 instanceof yz) {
            return this.a(ro.a((yz)sa2), 0.0f);
        }
        return false;
    }

    @Override
    public void i(int n2) {
        this.o.X();
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (!this.K && !this.o.E) {
            this.B();
            this.Q();
            this.b(ro2.j());
        }
        return true;
    }

    @Override
    public void d(double d2, double d3, double d4) {
        if (!this.o.E && !this.K && d2 * d2 + d3 * d3 + d4 * d4 > 0.0) {
            this.B();
            this.b((sa)null);
        }
    }

    @Override
    public void g(double d2, double d3, double d4) {
        if (!this.o.E && !this.K && d2 * d2 + d3 * d3 + d4 * d4 > 0.0) {
            this.B();
            this.b((sa)null);
        }
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Direction", (byte)this.a);
        dh2.a("TileX", this.b);
        dh2.a("TileY", this.c);
        dh2.a("TileZ", this.d);
        switch (this.a) {
            case 2: {
                dh2.a("Dir", (byte)0);
                break;
            }
            case 1: {
                dh2.a("Dir", (byte)1);
                break;
            }
            case 0: {
                dh2.a("Dir", (byte)2);
                break;
            }
            case 3: {
                dh2.a("Dir", (byte)3);
            }
        }
    }

    @Override
    public void a(dh dh2) {
        if (dh2.b("Direction", 99)) {
            this.a = dh2.d("Direction");
        } else {
            switch (dh2.d("Dir")) {
                case 0: {
                    this.a = 2;
                    break;
                }
                case 1: {
                    this.a = 1;
                    break;
                }
                case 2: {
                    this.a = 0;
                    break;
                }
                case 3: {
                    this.a = 3;
                }
            }
        }
        this.b = dh2.f("TileX");
        this.c = dh2.f("TileY");
        this.d = dh2.f("TileZ");
        this.a(this.a);
    }

    public abstract int f();

    public abstract int i();

    public abstract void b(sa var1);

    @Override
    protected boolean V() {
        return false;
    }
}

