/*
 * Decompiled with CFR 0.152.
 */
public class ur
extends ui {
    private td a;
    private vy b;
    private int c = -1;
    private int d = -1;

    public ur(td td2) {
        this.a = td2;
        this.a(1);
    }

    @Override
    public boolean a() {
        int n2 = qh.c(this.a.s);
        int n3 = qh.c(this.a.t);
        int n4 = qh.c(this.a.u);
        if (this.a.o.w() && !this.a.o.Q() && this.a.o.a(n2, n4).e() || this.a.o.t.g) {
            return false;
        }
        if (this.a.aI().nextInt(50) != 0) {
            return false;
        }
        if (this.c != -1 && this.a.e(this.c, this.a.t, this.d) < 4.0) {
            return false;
        }
        vz vz2 = this.a.o.A.a(n2, n3, n4, 14);
        if (vz2 == null) {
            return false;
        }
        this.b = vz2.c(n2, n3, n4);
        return this.b != null;
    }

    @Override
    public boolean b() {
        return !this.a.m().g();
    }

    @Override
    public void c() {
        this.c = -1;
        if (this.a.e(this.b.a(), this.b.b, this.b.c()) > 256.0) {
            azw azw2 = vx.a(this.a, 14, 3, azw.a((double)this.b.a() + 0.5, this.b.b(), (double)this.b.c() + 0.5));
            if (azw2 != null) {
                this.a.m().a(azw2.a, azw2.b, azw2.c, 1.0);
            }
        } else {
            this.a.m().a((double)this.b.a() + 0.5, this.b.b(), (double)this.b.c() + 0.5, 1.0);
        }
    }

    @Override
    public void d() {
        this.c = this.b.a();
        this.d = this.b.c();
        this.b = null;
    }
}

