/*
 * Decompiled with CFR 0.152.
 */
import org.apache.commons.lang3.StringUtils;

public abstract class vu
extends ui {
    protected td c;
    protected boolean d;
    private boolean a;
    private int b;
    private int e;
    private int f;

    public vu(td td2, boolean bl2) {
        this(td2, bl2, false);
    }

    public vu(td td2, boolean bl2, boolean bl3) {
        this.c = td2;
        this.d = bl2;
        this.a = bl3;
    }

    @Override
    public boolean b() {
        sv sv2 = this.c.o();
        if (sv2 == null) {
            return false;
        }
        if (!sv2.Z()) {
            return false;
        }
        double d2 = this.f();
        if (this.c.f(sv2) > d2 * d2) {
            return false;
        }
        if (this.d) {
            if (this.c.n().a(sv2)) {
                this.f = 0;
            } else if (++this.f > 60) {
                return false;
            }
        }
        return !(sv2 instanceof mw) || !((mw)sv2).c.d();
    }

    protected double f() {
        ti ti2 = this.c.a(yj.b);
        return ti2 == null ? 16.0 : ti2.e();
    }

    @Override
    public void c() {
        this.b = 0;
        this.e = 0;
        this.f = 0;
    }

    @Override
    public void d() {
        this.c.d((sv)null);
    }

    protected boolean a(sv sv2, boolean bl2) {
        if (sv2 == null) {
            return false;
        }
        if (sv2 == this.c) {
            return false;
        }
        if (!sv2.Z()) {
            return false;
        }
        if (!this.c.a(sv2.getClass())) {
            return false;
        }
        if (this.c instanceof ta && StringUtils.isNotEmpty(((ta)((Object)this.c)).b())) {
            if (sv2 instanceof ta && ((ta)((Object)this.c)).b().equals(((ta)((Object)sv2)).b())) {
                return false;
            }
            if (sv2 == ((ta)((Object)this.c)).i_()) {
                return false;
            }
        } else if (sv2 instanceof yz && !bl2 && ((yz)sv2).bE.a) {
            return false;
        }
        if (!this.c.b(qh.c(sv2.s), qh.c(sv2.t), qh.c(sv2.u))) {
            return false;
        }
        if (this.d && !this.c.n().a(sv2)) {
            return false;
        }
        if (this.a) {
            if (--this.e <= 0) {
                this.b = 0;
            }
            if (this.b == 0) {
                int n2 = this.b = this.a(sv2) ? 1 : 2;
            }
            if (this.b == 2) {
                return false;
            }
        }
        return true;
    }

    private boolean a(sv sv2) {
        int n2;
        this.e = 10 + this.c.aI().nextInt(5);
        ayf ayf2 = this.c.m().a(sv2);
        if (ayf2 == null) {
            return false;
        }
        aye aye2 = ayf2.c();
        if (aye2 == null) {
            return false;
        }
        int n3 = aye2.a - qh.c(sv2.s);
        return (double)(n3 * n3 + (n2 = aye2.c - qh.c(sv2.u)) * n2) <= 2.25;
    }
}

