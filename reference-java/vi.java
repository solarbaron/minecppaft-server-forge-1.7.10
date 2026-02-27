/*
 * Decompiled with CFR 0.152.
 */
public class vi
extends ui {
    xz a;
    sv b;

    public vi(xz xz2) {
        this.a = xz2;
        this.a(1);
    }

    @Override
    public boolean a() {
        sv sv2 = this.a.o();
        return this.a.cb() > 0 || sv2 != null && this.a.f(sv2) < 9.0;
    }

    @Override
    public void c() {
        this.a.m().h();
        this.b = this.a.o();
    }

    @Override
    public void d() {
        this.b = null;
    }

    @Override
    public void e() {
        if (this.b == null) {
            this.a.a(-1);
            return;
        }
        if (this.a.f(this.b) > 49.0) {
            this.a.a(-1);
            return;
        }
        if (!this.a.n().a(this.b)) {
            this.a.a(-1);
            return;
        }
        this.a.a(1);
    }
}

