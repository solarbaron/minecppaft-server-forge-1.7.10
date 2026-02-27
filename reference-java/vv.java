/*
 * Decompiled with CFR 0.152.
 */
public class vv {
    private sw a;
    private ahb b;
    private ayf c;
    private double d;
    private ti e;
    private boolean f;
    private int g;
    private int h;
    private azw i = azw.a(0.0, 0.0, 0.0);
    private boolean j = true;
    private boolean k;
    private boolean l;
    private boolean m;

    public vv(sw sw2, ahb ahb2) {
        this.a = sw2;
        this.b = ahb2;
        this.e = sw2.a(yj.b);
    }

    public void a(boolean bl2) {
        this.l = bl2;
    }

    public boolean a() {
        return this.l;
    }

    public void b(boolean bl2) {
        this.k = bl2;
    }

    public void c(boolean bl2) {
        this.j = bl2;
    }

    public boolean c() {
        return this.k;
    }

    public void d(boolean bl2) {
        this.f = bl2;
    }

    public void a(double d2) {
        this.d = d2;
    }

    public void e(boolean bl2) {
        this.m = bl2;
    }

    public float d() {
        return (float)this.e.e();
    }

    public ayf a(double d2, double d3, double d4) {
        if (!this.l()) {
            return null;
        }
        return this.b.a(this.a, qh.c(d2), (int)d3, qh.c(d4), this.d(), this.j, this.k, this.l, this.m);
    }

    public boolean a(double d2, double d3, double d4, double d5) {
        ayf ayf2 = this.a(qh.c(d2), (int)d3, qh.c(d4));
        return this.a(ayf2, d5);
    }

    public ayf a(sa sa2) {
        if (!this.l()) {
            return null;
        }
        return this.b.a((sa)this.a, sa2, this.d(), this.j, this.k, this.l, this.m);
    }

    public boolean a(sa sa2, double d2) {
        ayf ayf2 = this.a(sa2);
        if (ayf2 != null) {
            return this.a(ayf2, d2);
        }
        return false;
    }

    public boolean a(ayf ayf2, double d2) {
        if (ayf2 == null) {
            this.c = null;
            return false;
        }
        if (!ayf2.a(this.c)) {
            this.c = ayf2;
        }
        if (this.f) {
            this.n();
        }
        if (this.c.d() == 0) {
            return false;
        }
        this.d = d2;
        azw azw2 = this.j();
        this.h = this.g;
        this.i.a = azw2.a;
        this.i.b = azw2.b;
        this.i.c = azw2.c;
        return true;
    }

    public ayf e() {
        return this.c;
    }

    public void f() {
        ++this.g;
        if (this.g()) {
            return;
        }
        if (this.l()) {
            this.i();
        }
        if (this.g()) {
            return;
        }
        azw azw2 = this.c.a(this.a);
        if (azw2 == null) {
            return;
        }
        this.a.k().a(azw2.a, azw2.b, azw2.c, this.d);
    }

    private void i() {
        int n2;
        azw azw2 = this.j();
        int n3 = this.c.d();
        for (int i2 = this.c.e(); i2 < this.c.d(); ++i2) {
            if (this.c.a((int)i2).b == (int)azw2.b) continue;
            n3 = i2;
            break;
        }
        float f2 = this.a.M * this.a.M;
        for (n2 = this.c.e(); n2 < n3; ++n2) {
            if (!(azw2.e(this.c.a(this.a, n2)) < (double)f2)) continue;
            this.c.c(n2 + 1);
        }
        n2 = qh.f(this.a.M);
        int n4 = (int)this.a.N + 1;
        int n5 = n2;
        for (int i3 = n3 - 1; i3 >= this.c.e(); --i3) {
            if (!this.a(azw2, this.c.a(this.a, i3), n2, n4, n5)) continue;
            this.c.c(i3);
            break;
        }
        if (this.g - this.h > 100) {
            if (azw2.e(this.i) < 2.25) {
                this.h();
            }
            this.h = this.g;
            this.i.a = azw2.a;
            this.i.b = azw2.b;
            this.i.c = azw2.c;
        }
    }

    public boolean g() {
        return this.c == null || this.c.b();
    }

    public void h() {
        this.c = null;
    }

    private azw j() {
        return azw.a(this.a.s, this.k(), this.a.u);
    }

    private int k() {
        if (!this.a.M() || !this.m) {
            return (int)(this.a.C.b + 0.5);
        }
        int n2 = (int)this.a.C.b;
        aji aji2 = this.b.a(qh.c(this.a.s), n2, qh.c(this.a.u));
        int n3 = 0;
        while (aji2 == ajn.i || aji2 == ajn.j) {
            aji2 = this.b.a(qh.c(this.a.s), ++n2, qh.c(this.a.u));
            if (++n3 <= 16) continue;
            return (int)this.a.C.b;
        }
        return n2;
    }

    private boolean l() {
        return this.a.D || this.m && this.m() || this.a.am() && this.a instanceof yq && this.a.m instanceof wg;
    }

    private boolean m() {
        return this.a.M() || this.a.P();
    }

    private void n() {
        if (this.b.i(qh.c(this.a.s), (int)(this.a.C.b + 0.5), qh.c(this.a.u))) {
            return;
        }
        for (int i2 = 0; i2 < this.c.d(); ++i2) {
            aye aye2 = this.c.a(i2);
            if (!this.b.i(aye2.a, aye2.b, aye2.c)) continue;
            this.c.b(i2 - 1);
            return;
        }
    }

    private boolean a(azw azw2, azw azw3, int n2, int n3, int n4) {
        int n5 = qh.c(azw2.a);
        int n6 = qh.c(azw2.c);
        double d2 = azw3.a - azw2.a;
        double d3 = azw3.c - azw2.c;
        double d4 = d2 * d2 + d3 * d3;
        if (d4 < 1.0E-8) {
            return false;
        }
        double d5 = 1.0 / Math.sqrt(d4);
        if (!this.a(n5, (int)azw2.b, n6, n2 += 2, n3, n4 += 2, azw2, d2 *= d5, d3 *= d5)) {
            return false;
        }
        n2 -= 2;
        n4 -= 2;
        double d6 = 1.0 / Math.abs(d2);
        double d7 = 1.0 / Math.abs(d3);
        double d8 = (double)(n5 * 1) - azw2.a;
        double d9 = (double)(n6 * 1) - azw2.c;
        if (d2 >= 0.0) {
            d8 += 1.0;
        }
        if (d3 >= 0.0) {
            d9 += 1.0;
        }
        d8 /= d2;
        d9 /= d3;
        int n7 = d2 < 0.0 ? -1 : 1;
        int n8 = d3 < 0.0 ? -1 : 1;
        int n9 = qh.c(azw3.a);
        int n10 = qh.c(azw3.c);
        int n11 = n9 - n5;
        int n12 = n10 - n6;
        while (n11 * n7 > 0 || n12 * n8 > 0) {
            if (d8 < d9) {
                d8 += d6;
                n11 = n9 - (n5 += n7);
            } else {
                d9 += d7;
                n12 = n10 - (n6 += n8);
            }
            if (this.a(n5, (int)azw2.b, n6, n2, n3, n4, azw2, d2, d3)) continue;
            return false;
        }
        return true;
    }

    private boolean a(int n2, int n3, int n4, int n5, int n6, int n7, azw azw2, double d2, double d3) {
        int n8 = n2 - n5 / 2;
        int n9 = n4 - n7 / 2;
        if (!this.b(n8, n3, n9, n5, n6, n7, azw2, d2, d3)) {
            return false;
        }
        for (int i2 = n8; i2 < n8 + n5; ++i2) {
            for (int i3 = n9; i3 < n9 + n7; ++i3) {
                double d4 = (double)i2 + 0.5 - azw2.a;
                double d5 = (double)i3 + 0.5 - azw2.c;
                if (d4 * d2 + d5 * d3 < 0.0) continue;
                aji aji2 = this.b.a(i2, n3 - 1, i3);
                awt awt2 = aji2.o();
                if (awt2 == awt.a) {
                    return false;
                }
                if (awt2 == awt.h && !this.a.M()) {
                    return false;
                }
                if (awt2 != awt.i) continue;
                return false;
            }
        }
        return true;
    }

    private boolean b(int n2, int n3, int n4, int n5, int n6, int n7, azw azw2, double d2, double d3) {
        for (int i2 = n2; i2 < n2 + n5; ++i2) {
            for (int i3 = n3; i3 < n3 + n6; ++i3) {
                for (int i4 = n4; i4 < n4 + n7; ++i4) {
                    aji aji2;
                    double d4 = (double)i2 + 0.5 - azw2.a;
                    double d5 = (double)i4 + 0.5 - azw2.c;
                    if (d4 * d2 + d5 * d3 < 0.0 || (aji2 = this.b.a(i2, i3, i4)).b((ahl)this.b, i2, i3, i4)) continue;
                    return false;
                }
            }
        }
        return true;
    }
}

