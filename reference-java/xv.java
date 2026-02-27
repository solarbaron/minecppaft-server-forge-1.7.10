/*
 * Decompiled with CFR 0.152.
 */
public class xv
extends xl {
    private int a = -1;

    public xv(ahb ahb2) {
        super(ahb2);
    }

    public xv(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    public int m() {
        return 3;
    }

    @Override
    public aji o() {
        return ajn.W;
    }

    @Override
    public void h() {
        double d2;
        super.h();
        if (this.a > 0) {
            --this.a;
            this.o.a("smoke", this.s, this.t + 0.5, this.u, 0.0, 0.0, 0.0);
        } else if (this.a == 0) {
            this.c(this.v * this.v + this.x * this.x);
        }
        if (this.E && (d2 = this.v * this.v + this.x * this.x) >= (double)0.01f) {
            this.c(d2);
        }
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        double d2 = this.v * this.v + this.x * this.x;
        if (!ro2.c()) {
            this.a(new add(ajn.W, 1), 0.0f);
        }
        if (ro2.o() || ro2.c() || d2 >= (double)0.01f) {
            this.c(d2);
        }
    }

    protected void c(double d2) {
        if (!this.o.E) {
            double d3 = Math.sqrt(d2);
            if (d3 > 5.0) {
                d3 = 5.0;
            }
            this.o.a(this, this.s, this.t, this.u, (float)(4.0 + this.Z.nextDouble() * 1.5 * d3), true);
            this.B();
        }
    }

    @Override
    protected void b(float f2) {
        if (f2 >= 3.0f) {
            float f3 = f2 / 10.0f;
            this.c(f3 * f3);
        }
        super.b(f2);
    }

    @Override
    public void a(int n2, int n3, int n4, boolean bl2) {
        if (bl2 && this.a < 0) {
            this.e();
        }
    }

    public void e() {
        this.a = 80;
        if (!this.o.E) {
            this.o.a((sa)this, (byte)10);
            this.o.a(this, "game.tnt.primed", 1.0f, 1.0f);
        }
    }

    public boolean v() {
        return this.a > -1;
    }

    @Override
    public float a(agw agw2, ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (this.v() && (aje.a(aji2) || aje.b_(ahb2, n2, n3 + 1, n4))) {
            return 0.0f;
        }
        return super.a(agw2, ahb2, n2, n3, n4, aji2);
    }

    @Override
    public boolean a(agw agw2, ahb ahb2, int n2, int n3, int n4, aji aji2, float f2) {
        if (this.v() && (aje.a(aji2) || aje.b_(ahb2, n2, n3 + 1, n4))) {
            return false;
        }
        return super.a(agw2, ahb2, n2, n3, n4, aji2, f2);
    }

    @Override
    protected void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("TNTFuse", 99)) {
            this.a = dh2.f("TNTFuse");
        }
    }

    @Override
    protected void b(dh dh2) {
        super.b(dh2);
        dh2.a("TNTFuse", this.a);
    }
}

