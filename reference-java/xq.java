/*
 * Decompiled with CFR 0.152.
 */
public class xq
extends xl {
    private int c;
    public double a;
    public double b;

    public xq(ahb ahb2) {
        super(ahb2);
    }

    public xq(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    public int m() {
        return 2;
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
    }

    @Override
    public void h() {
        super.h();
        if (this.c > 0) {
            --this.c;
        }
        if (this.c <= 0) {
            this.b = 0.0;
            this.a = 0.0;
        }
        this.f(this.c > 0);
        if (this.e() && this.Z.nextInt(4) == 0) {
            this.o.a("largesmoke", this.s, this.t + 0.8, this.u, 0.0, 0.0, 0.0);
        }
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        if (!ro2.c()) {
            this.a(new add(ajn.al, 1), 0.0f);
        }
    }

    @Override
    protected void a(int n2, int n3, int n4, double d2, double d3, aji aji2, int n5) {
        super.a(n2, n3, n4, d2, d3, aji2, n5);
        double d4 = this.a * this.a + this.b * this.b;
        if (d4 > 1.0E-4 && this.v * this.v + this.x * this.x > 0.001) {
            d4 = qh.a(d4);
            this.a /= d4;
            this.b /= d4;
            if (this.a * this.v + this.b * this.x < 0.0) {
                this.a = 0.0;
                this.b = 0.0;
            } else {
                this.a = this.v;
                this.b = this.x;
            }
        }
    }

    @Override
    protected void i() {
        double d2 = this.a * this.a + this.b * this.b;
        if (d2 > 1.0E-4) {
            d2 = qh.a(d2);
            this.a /= d2;
            this.b /= d2;
            double d3 = 0.05;
            this.v *= (double)0.8f;
            this.w *= 0.0;
            this.x *= (double)0.8f;
            this.v += this.a * d3;
            this.x += this.b * d3;
        } else {
            this.v *= (double)0.98f;
            this.w *= 0.0;
            this.x *= (double)0.98f;
        }
        super.i();
    }

    @Override
    public boolean c(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.h) {
            if (!yz2.bE.d && --add2.b == 0) {
                yz2.bm.a(yz2.bm.c, null);
            }
            this.c += 3600;
        }
        this.a = this.s - yz2.s;
        this.b = this.u - yz2.u;
        return true;
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        dh2.a("PushX", this.a);
        dh2.a("PushZ", this.b);
        dh2.a("Fuel", (short)this.c);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        this.a = dh2.i("PushX");
        this.b = dh2.i("PushZ");
        this.c = dh2.e("Fuel");
    }

    protected boolean e() {
        return (this.af.a(16) & 1) != 0;
    }

    protected void f(boolean bl2) {
        if (bl2) {
            this.af.b(16, (byte)(this.af.a(16) | 1));
        } else {
            this.af.b(16, (byte)(this.af.a(16) & 0xFFFFFFFE));
        }
    }

    @Override
    public aji o() {
        return ajn.am;
    }

    @Override
    public int q() {
        return 2;
    }
}

