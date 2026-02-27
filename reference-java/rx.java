/*
 * Decompiled with CFR 0.152.
 */
public abstract class rx
extends td {
    private float bp = -1.0f;
    private float bq;

    public rx(ahb ahb2) {
        super(ahb2);
    }

    public abstract rx a(rx var1);

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.bx) {
            rx rx2;
            Class clazz;
            if (!this.o.E && (clazz = sg.a(add2.k())) != null && clazz.isAssignableFrom(this.getClass()) && (rx2 = this.a(this)) != null) {
                rx2.c(-24000);
                rx2.b(this.s, this.t, this.u, 0.0f, 0.0f);
                this.o.d(rx2);
                if (add2.u()) {
                    rx2.a(add2.s());
                }
                if (!yz2.bE.d) {
                    --add2.b;
                    if (add2.b <= 0) {
                        yz2.bm.a(yz2.bm.c, null);
                    }
                }
            }
            return true;
        }
        return false;
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(12, new Integer(0));
    }

    public int d() {
        return this.af.c(12);
    }

    public void a(int n2) {
        int n3 = this.d();
        if ((n3 += n2 * 20) > 0) {
            n3 = 0;
        }
        this.c(n3);
    }

    public void c(int n2) {
        this.af.b(12, n2);
        this.a(this.f());
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Age", this.d());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.c(dh2.f("Age"));
    }

    @Override
    public void e() {
        super.e();
        if (this.o.E) {
            this.a(this.f());
        } else {
            int n2 = this.d();
            if (n2 < 0) {
                this.c(++n2);
            } else if (n2 > 0) {
                this.c(--n2);
            }
        }
    }

    @Override
    public boolean f() {
        return this.d() < 0;
    }

    public void a(boolean bl2) {
        this.a(bl2 ? 0.5f : 1.0f);
    }

    @Override
    protected final void a(float f2, float f3) {
        boolean bl2 = this.bp > 0.0f;
        this.bp = f2;
        this.bq = f3;
        if (!bl2) {
            this.a(1.0f);
        }
    }

    protected final void a(float f2) {
        super.a(this.bp * f2, this.bq * f2);
    }
}

