/*
 * Decompiled with CFR 0.152.
 */
public class vt
extends vu {
    tg a;
    sv b;
    private int e;

    public vt(tg tg2) {
        super(tg2, false);
        this.a = tg2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (!this.a.bZ()) {
            return false;
        }
        sv sv2 = this.a.cb();
        if (sv2 == null) {
            return false;
        }
        this.b = sv2.aL();
        int n2 = sv2.aM();
        return n2 != this.e && this.a(this.b, false) && this.a.a(this.b, sv2);
    }

    @Override
    public void c() {
        this.c.d(this.b);
        sv sv2 = this.a.cb();
        if (sv2 != null) {
            this.e = sv2.aM();
        }
        super.c();
    }
}

