/*
 * Decompiled with CFR 0.152.
 */
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class xk
extends sa {
    private static final Logger d = LogManager.getLogger();
    public int a;
    public int b;
    private int e = 5;
    private String f;
    private String g;
    public float c = (float)(Math.random() * Math.PI * 2.0);

    public xk(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2);
        this.a(0.25f, 0.25f);
        this.L = this.N / 2.0f;
        this.b(d2, d3, d4);
        this.y = (float)(Math.random() * 360.0);
        this.v = (float)(Math.random() * (double)0.2f - (double)0.1f);
        this.w = 0.2f;
        this.x = (float)(Math.random() * (double)0.2f - (double)0.1f);
    }

    public xk(ahb ahb2, double d2, double d3, double d4, add add2) {
        this(ahb2, d2, d3, d4);
        this.a(add2);
    }

    @Override
    protected boolean g_() {
        return false;
    }

    public xk(ahb ahb2) {
        super(ahb2);
        this.a(0.25f, 0.25f);
        this.L = this.N / 2.0f;
    }

    @Override
    protected void c() {
        this.z().a(10, 5);
    }

    @Override
    public void h() {
        boolean bl2;
        if (this.f() == null) {
            this.B();
            return;
        }
        super.h();
        if (this.b > 0) {
            --this.b;
        }
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        this.w -= (double)0.04f;
        this.X = this.j(this.s, (this.C.b + this.C.e) / 2.0, this.u);
        this.d(this.v, this.w, this.x);
        boolean bl3 = bl2 = (int)this.p != (int)this.s || (int)this.q != (int)this.t || (int)this.r != (int)this.u;
        if (bl2 || this.aa % 25 == 0) {
            if (this.o.a(qh.c(this.s), qh.c(this.t), qh.c(this.u)).o() == awt.i) {
                this.w = 0.2f;
                this.v = (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f;
                this.x = (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f;
                this.a("random.fizz", 0.4f, 2.0f + this.Z.nextFloat() * 0.4f);
            }
            if (!this.o.E) {
                this.k();
            }
        }
        float f2 = 0.98f;
        if (this.D) {
            f2 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.98f;
        }
        this.v *= (double)f2;
        this.w *= (double)0.98f;
        this.x *= (double)f2;
        if (this.D) {
            this.w *= -0.5;
        }
        ++this.a;
        if (!this.o.E && this.a >= 6000) {
            this.B();
        }
    }

    private void k() {
        for (xk xk2 : this.o.a(xk.class, this.C.b(0.5, 0.0, 0.5))) {
            this.a(xk2);
        }
    }

    public boolean a(xk xk2) {
        if (xk2 == this) {
            return false;
        }
        if (!xk2.Z() || !this.Z()) {
            return false;
        }
        add add2 = this.f();
        add add3 = xk2.f();
        if (add3.b() != add2.b()) {
            return false;
        }
        if (add3.p() ^ add2.p()) {
            return false;
        }
        if (add3.p() && !add3.q().equals(add2.q())) {
            return false;
        }
        if (add3.b() == null) {
            return false;
        }
        if (add3.b().n() && add3.k() != add2.k()) {
            return false;
        }
        if (add3.b < add2.b) {
            return xk2.a(this);
        }
        if (add3.b + add2.b > add3.e()) {
            return false;
        }
        add3.b += add2.b;
        xk2.b = Math.max(xk2.b, this.b);
        xk2.a = Math.min(xk2.a, this.a);
        xk2.a(add3);
        this.B();
        return true;
    }

    public void e() {
        this.a = 4800;
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
        if (this.f() != null && this.f().b() == ade.bN && ro2.c()) {
            return false;
        }
        this.Q();
        this.e = (int)((float)this.e - f2);
        if (this.e <= 0) {
            this.B();
        }
        return false;
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Health", (short)((byte)this.e));
        dh2.a("Age", (short)this.a);
        if (this.j() != null) {
            dh2.a("Thrower", this.f);
        }
        if (this.i() != null) {
            dh2.a("Owner", this.g);
        }
        if (this.f() != null) {
            dh2.a("Item", this.f().b(new dh()));
        }
    }

    @Override
    public void a(dh dh2) {
        this.e = dh2.e("Health") & 0xFF;
        this.a = dh2.e("Age");
        if (dh2.c("Owner")) {
            this.g = dh2.j("Owner");
        }
        if (dh2.c("Thrower")) {
            this.f = dh2.j("Thrower");
        }
        dh dh3 = dh2.m("Item");
        this.a(add.a(dh3));
        if (this.f() == null) {
            this.B();
        }
    }

    @Override
    public void b_(yz yz2) {
        if (this.o.E) {
            return;
        }
        add add2 = this.f();
        int n2 = add2.b;
        if (this.b == 0 && (this.g == null || 6000 - this.a <= 200 || this.g.equals(yz2.b_())) && yz2.bm.a(add2)) {
            yz yz3;
            if (add2.b() == adb.a(ajn.r)) {
                yz2.a(pc.g);
            }
            if (add2.b() == adb.a(ajn.s)) {
                yz2.a(pc.g);
            }
            if (add2.b() == ade.aA) {
                yz2.a(pc.t);
            }
            if (add2.b() == ade.i) {
                yz2.a(pc.w);
            }
            if (add2.b() == ade.bj) {
                yz2.a(pc.A);
            }
            if (add2.b() == ade.i && this.j() != null && (yz3 = this.o.a(this.j())) != null && yz3 != yz2) {
                yz3.a(pc.x);
            }
            this.o.a((sa)yz2, "random.pop", 0.2f, ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.7f + 1.0f) * 2.0f);
            yz2.a((sa)this, n2);
            if (add2.b <= 0) {
                this.B();
            }
        }
    }

    @Override
    public String b_() {
        return dd.a("item." + this.f().a());
    }

    @Override
    public boolean av() {
        return false;
    }

    @Override
    public void b(int n2) {
        super.b(n2);
        if (!this.o.E) {
            this.k();
        }
    }

    public add f() {
        add add2 = this.z().f(10);
        if (add2 == null) {
            return new add(ajn.b);
        }
        return add2;
    }

    public void a(add add2) {
        this.z().b(10, add2);
        this.z().h(10);
    }

    public String i() {
        return this.g;
    }

    public void a(String string) {
        this.g = string;
    }

    public String j() {
        return this.f;
    }

    public void b(String string) {
        this.f = string;
    }
}

