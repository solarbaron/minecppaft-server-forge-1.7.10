/*
 * Decompiled with CFR 0.152.
 */
public abstract class xp
extends xl
implements rb {
    private add[] a = new add[36];
    private boolean b = true;

    public xp(ahb ahb2) {
        super(ahb2);
    }

    public xp(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        for (int i2 = 0; i2 < this.a(); ++i2) {
            add add2 = this.a(i2);
            if (add2 == null) continue;
            float f2 = this.Z.nextFloat() * 0.8f + 0.1f;
            float f3 = this.Z.nextFloat() * 0.8f + 0.1f;
            float f4 = this.Z.nextFloat() * 0.8f + 0.1f;
            while (add2.b > 0) {
                int n2 = this.Z.nextInt(21) + 10;
                if (n2 > add2.b) {
                    n2 = add2.b;
                }
                add2.b -= n2;
                xk xk2 = new xk(this.o, this.s + (double)f2, this.t + (double)f3, this.u + (double)f4, new add(add2.b(), n2, add2.k()));
                float f5 = 0.05f;
                xk2.v = (float)this.Z.nextGaussian() * f5;
                xk2.w = (float)this.Z.nextGaussian() * f5 + 0.2f;
                xk2.x = (float)this.Z.nextGaussian() * f5;
                this.o.d(xk2);
            }
        }
    }

    @Override
    public add a(int n2) {
        return this.a[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.a[n2] != null) {
            if (this.a[n2].b <= n3) {
                add add2 = this.a[n2];
                this.a[n2] = null;
                return add2;
            }
            add add3 = this.a[n2].a(n3);
            if (this.a[n2].b == 0) {
                this.a[n2] = null;
            }
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.a[n2] != null) {
            add add2 = this.a[n2];
            this.a[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.a[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
    }

    @Override
    public void e() {
    }

    @Override
    public boolean a(yz yz2) {
        if (this.K) {
            return false;
        }
        return !(yz2.f(this) > 64.0);
    }

    @Override
    public void f() {
    }

    @Override
    public void l_() {
    }

    @Override
    public boolean b(int n2, add add2) {
        return true;
    }

    @Override
    public String b() {
        return this.k_() ? this.u() : "container.minecart";
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public void b(int n2) {
        this.b = false;
        super.b(n2);
    }

    @Override
    public void B() {
        if (this.b) {
            for (int i2 = 0; i2 < this.a(); ++i2) {
                add add2 = this.a(i2);
                if (add2 == null) continue;
                float f2 = this.Z.nextFloat() * 0.8f + 0.1f;
                float f3 = this.Z.nextFloat() * 0.8f + 0.1f;
                float f4 = this.Z.nextFloat() * 0.8f + 0.1f;
                while (add2.b > 0) {
                    int n2 = this.Z.nextInt(21) + 10;
                    if (n2 > add2.b) {
                        n2 = add2.b;
                    }
                    add2.b -= n2;
                    xk xk2 = new xk(this.o, this.s + (double)f2, this.t + (double)f3, this.u + (double)f4, new add(add2.b(), n2, add2.k()));
                    if (add2.p()) {
                        xk2.f().d((dh)add2.q().b());
                    }
                    float f5 = 0.05f;
                    xk2.v = (float)this.Z.nextGaussian() * f5;
                    xk2.w = (float)this.Z.nextGaussian() * f5 + 0.2f;
                    xk2.x = (float)this.Z.nextGaussian() * f5;
                    this.o.d(xk2);
                }
            }
        }
        super.B();
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.a[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.a = new add[this.a()];
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            int n2 = dh3.d("Slot") & 0xFF;
            if (n2 < 0 || n2 >= this.a.length) continue;
            this.a[n2] = add.a(dh3);
        }
    }

    @Override
    public boolean c(yz yz2) {
        if (!this.o.E) {
            yz2.a(this);
        }
        return true;
    }

    @Override
    protected void i() {
        int n2 = 15 - zs.b(this);
        float f2 = 0.98f + (float)n2 * 0.001f;
        this.v *= (double)f2;
        this.w *= 0.0;
        this.x *= (double)f2;
    }
}

