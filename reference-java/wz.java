/*
 * Decompiled with CFR 0.152.
 */
public class wz
extends sa {
    public int a;
    public int b;

    public wz(ahb ahb2) {
        super(ahb2);
        this.k = true;
        this.a(2.0f, 2.0f);
        this.L = this.N / 2.0f;
        this.b = 5;
        this.a = this.Z.nextInt(100000);
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    protected void c() {
        this.af.a(8, (Object)this.b);
    }

    @Override
    public void h() {
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        ++this.a;
        this.af.b(8, this.b);
        int n2 = qh.c(this.s);
        int n3 = qh.c(this.t);
        int n4 = qh.c(this.u);
        if (this.o.t instanceof aqr && this.o.a(n2, n3, n4) != ajn.ab) {
            this.o.b(n2, n3, n4, ajn.ab);
        }
    }

    @Override
    protected void b(dh dh2) {
    }

    @Override
    protected void a(dh dh2) {
    }

    @Override
    public boolean R() {
        return true;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (!this.K && !this.o.E) {
            this.b = 0;
            if (this.b <= 0) {
                this.B();
                if (!this.o.E) {
                    this.o.a(null, this.s, this.t, this.u, 6.0f, true);
                }
            }
        }
        return true;
    }
}

