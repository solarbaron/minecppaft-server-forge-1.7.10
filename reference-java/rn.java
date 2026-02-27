/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class rn {
    private final List a = new ArrayList();
    private final sv b;
    private int c;
    private int d;
    private int e;
    private boolean f;
    private boolean g;
    private String h;

    public rn(sv sv2) {
        this.b = sv2;
    }

    public void a() {
        this.j();
        if (this.b.h_()) {
            aji aji2 = this.b.o.a(qh.c(this.b.s), qh.c(this.b.C.b), qh.c(this.b.u));
            if (aji2 == ajn.ap) {
                this.h = "ladder";
            } else if (aji2 == ajn.bd) {
                this.h = "vines";
            }
        } else if (this.b.M()) {
            this.h = "water";
        }
    }

    public void a(ro ro2, float f2, float f3) {
        this.g();
        this.a();
        rm rm2 = new rm(ro2, this.b.aa, f2, f3, this.h, this.b.R);
        this.a.add(rm2);
        this.c = this.b.aa;
        this.g = true;
        if (rm2.f() && !this.f && this.b.Z()) {
            this.f = true;
            this.e = this.d = this.b.aa;
            this.b.bu();
        }
    }

    public fj b() {
        fj fj2;
        if (this.a.size() == 0) {
            return new fr("death.attack.generic", this.b.c_());
        }
        rm rm2 = this.i();
        rm rm3 = (rm)this.a.get(this.a.size() - 1);
        fj fj3 = rm3.h();
        sa sa2 = rm3.a().j();
        if (rm2 != null && rm3.a() == ro.h) {
            fj fj4 = rm2.h();
            if (rm2.a() == ro.h || rm2.a() == ro.i) {
                fj2 = new fr("death.fell.accident." + this.a(rm2), this.b.c_());
            } else if (!(fj4 == null || fj3 != null && fj4.equals(fj3))) {
                add add2;
                sa sa3 = rm2.a().j();
                add add3 = add2 = sa3 instanceof sv ? ((sv)sa3).be() : null;
                fj2 = add2 != null && add2.u() ? new fr("death.fell.assist.item", this.b.c_(), fj4, add2.E()) : new fr("death.fell.assist", this.b.c_(), fj4);
            } else if (fj3 != null) {
                add add4;
                add add5 = add4 = sa2 instanceof sv ? ((sv)sa2).be() : null;
                fj2 = add4 != null && add4.u() ? new fr("death.fell.finish.item", this.b.c_(), fj3, add4.E()) : new fr("death.fell.finish", this.b.c_(), fj3);
            } else {
                fj2 = new fr("death.fell.killer", this.b.c_());
            }
        } else {
            fj2 = rm3.a().b(this.b);
        }
        return fj2;
    }

    public sv c() {
        sv sv2 = null;
        yz yz2 = null;
        float f2 = 0.0f;
        float f3 = 0.0f;
        for (rm rm2 : this.a) {
            if (rm2.a().j() instanceof yz && (yz2 == null || rm2.c() > f3)) {
                f3 = rm2.c();
                yz2 = (yz)rm2.a().j();
            }
            if (!(rm2.a().j() instanceof sv) || sv2 != null && !(rm2.c() > f2)) continue;
            f2 = rm2.c();
            sv2 = (sv)rm2.a().j();
        }
        if (yz2 != null && f3 >= f2 / 3.0f) {
            return yz2;
        }
        return sv2;
    }

    private rm i() {
        rm rm2 = null;
        rm rm3 = null;
        int n2 = 0;
        float f2 = 0.0f;
        for (int i2 = 0; i2 < this.a.size(); ++i2) {
            rm rm4;
            rm rm5 = (rm)this.a.get(i2);
            rm rm6 = rm4 = i2 > 0 ? (rm)this.a.get(i2 - 1) : null;
            if ((rm5.a() == ro.h || rm5.a() == ro.i) && rm5.i() > 0.0f && (rm2 == null || rm5.i() > f2)) {
                rm2 = i2 > 0 ? rm4 : rm5;
                f2 = rm5.i();
            }
            if (rm5.g() == null || rm3 != null && !(rm5.c() > (float)n2)) continue;
            rm3 = rm5;
        }
        if (f2 > 5.0f && rm2 != null) {
            return rm2;
        }
        if (n2 > 5 && rm3 != null) {
            return rm3;
        }
        return null;
    }

    private String a(rm rm2) {
        return rm2.g() == null ? "generic" : rm2.g();
    }

    private void j() {
        this.h = null;
    }

    public void g() {
        int n2;
        int n3 = n2 = this.f ? 300 : 100;
        if (this.g && (!this.b.Z() || this.b.aa - this.c > n2)) {
            boolean bl2 = this.f;
            this.g = false;
            this.f = false;
            this.e = this.b.aa;
            if (bl2) {
                this.b.bv();
            }
            this.a.clear();
        }
    }
}

