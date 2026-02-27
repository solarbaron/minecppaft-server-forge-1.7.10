/*
 * Decompiled with CFR 0.152.
 */
public class vh
extends ui {
    private tg a;
    private boolean b;

    public vh(tg tg2) {
        this.a = tg2;
        this.a(5);
    }

    @Override
    public boolean a() {
        if (!this.a.bZ()) {
            return false;
        }
        if (this.a.M()) {
            return false;
        }
        if (!this.a.D) {
            return false;
        }
        sv sv2 = this.a.cb();
        if (sv2 == null) {
            return true;
        }
        if (this.a.f(sv2) < 144.0 && sv2.aJ() != null) {
            return false;
        }
        return this.b;
    }

    @Override
    public void c() {
        this.a.m().h();
        this.a.k(true);
    }

    @Override
    public void d() {
        this.a.k(false);
    }

    public void a(boolean bl2) {
        this.b = bl2;
    }
}

