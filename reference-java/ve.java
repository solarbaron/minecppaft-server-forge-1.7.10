/*
 * Decompiled with CFR 0.152.
 */
public class ve
extends ui {
    private td a;
    private vy b;

    public ve(td td2) {
        this.a = td2;
    }

    @Override
    public boolean a() {
        if (this.a.o.w()) {
            return false;
        }
        vz vz2 = this.a.o.A.a(qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u), 16);
        if (vz2 == null) {
            return false;
        }
        this.b = vz2.b(qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u));
        if (this.b == null) {
            return false;
        }
        return (double)this.b.c(qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u)) < 2.25;
    }

    @Override
    public boolean b() {
        if (this.a.o.w()) {
            return false;
        }
        return !this.b.g && this.b.a(qh.c(this.a.s), qh.c(this.a.u));
    }

    @Override
    public void c() {
        this.a.m().b(false);
        this.a.m().c(false);
    }

    @Override
    public void d() {
        this.a.m().b(true);
        this.a.m().c(true);
        this.b = null;
    }

    @Override
    public void e() {
        this.b.e();
    }
}

