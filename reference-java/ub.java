/*
 * Decompiled with CFR 0.152.
 */
public class ub
extends ui {
    private final sw a;
    private final float b;
    private float c;
    private boolean d;
    private int e;
    private int f;

    public ub(sw sw2, float f2) {
        this.a = sw2;
        this.b = f2;
        this.a(7);
    }

    @Override
    public void c() {
        this.c = 0.0f;
    }

    @Override
    public void d() {
        this.d = false;
        this.c = 0.0f;
    }

    @Override
    public boolean a() {
        return this.a.Z() && this.a.l != null && this.a.l instanceof yz && (this.d || this.a.bE());
    }

    @Override
    public void e() {
        Object object;
        yz yz2 = (yz)this.a.l;
        td td2 = (td)this.a;
        float f2 = qh.g(yz2.y - this.a.y) * 0.5f;
        if (f2 > 5.0f) {
            f2 = 5.0f;
        }
        if (f2 < -5.0f) {
            f2 = -5.0f;
        }
        this.a.y = qh.g(this.a.y + f2);
        if (this.c < this.b) {
            this.c += (this.b - this.c) * 0.01f;
        }
        if (this.c > this.b) {
            this.c = this.b;
        }
        int n2 = qh.c(this.a.s);
        int n3 = qh.c(this.a.t);
        int n4 = qh.c(this.a.u);
        float f3 = this.c;
        if (this.d) {
            if (this.e++ > this.f) {
                this.d = false;
            }
            f3 += f3 * 1.15f * qh.a((float)this.e / (float)this.f * (float)Math.PI);
        }
        float f4 = 0.91f;
        if (this.a.D) {
            f4 = this.a.o.a((int)qh.d((float)((float)n2)), (int)(qh.d((float)((float)n3)) - 1), (int)qh.d((float)((float)n4))).K * 0.91f;
        }
        float f5 = 0.16277136f / (f4 * f4 * f4);
        float f6 = qh.a(td2.y * (float)Math.PI / 180.0f);
        float f7 = qh.b(td2.y * (float)Math.PI / 180.0f);
        float f8 = td2.bl() * f5;
        float f9 = Math.max(f3, 1.0f);
        f9 = f8 / f9;
        float f10 = f3 * f9;
        float f11 = -(f10 * f6);
        float f12 = f10 * f7;
        if (qh.e(f11) > qh.e(f12)) {
            if (f11 < 0.0f) {
                f11 -= this.a.M / 2.0f;
            }
            if (f11 > 0.0f) {
                f11 += this.a.M / 2.0f;
            }
            f12 = 0.0f;
        } else {
            f11 = 0.0f;
            if (f12 < 0.0f) {
                f12 -= this.a.M / 2.0f;
            }
            if (f12 > 0.0f) {
                f12 += this.a.M / 2.0f;
            }
        }
        int n5 = qh.c(this.a.s + (double)f11);
        int n6 = qh.c(this.a.u + (double)f12);
        aye aye2 = new aye(qh.d(this.a.M + 1.0f), qh.d(this.a.N + yz2.N + 1.0f), qh.d(this.a.M + 1.0f));
        if (n2 != n5 || n4 != n6) {
            boolean bl2;
            object = this.a.o.a(n2, n3, n4);
            boolean bl3 = bl2 = !this.a((aji)object) && (((aji)object).o() != awt.a || !this.a(this.a.o.a(n2, n3 - 1, n4)));
            if (bl2 && ayg.a(this.a, n5, n3, n6, aye2, false, false, true) == 0 && ayg.a(this.a, n2, n3 + 1, n4, aye2, false, false, true) == 1 && ayg.a(this.a, n5, n3 + 1, n6, aye2, false, false, true) == 1) {
                td2.l().a();
            }
        }
        if (!yz2.bE.d && this.c >= this.b * 0.5f && this.a.aI().nextFloat() < 0.006f && !this.d && (object = yz2.be()) != null && ((add)object).b() == ade.bM) {
            ((add)object).a(1, (sv)yz2);
            if (((add)object).b == 0) {
                add add2 = new add(ade.aM);
                add2.d(((add)object).d);
                yz2.bm.a[yz2.bm.c] = add2;
            }
        }
        this.a.e(0.0f, f3);
    }

    private boolean a(aji aji2) {
        return aji2.b() == 10 || aji2 instanceof alj;
    }

    public boolean f() {
        return this.d;
    }

    public void g() {
        this.d = true;
        this.e = 0;
        this.f = this.a.aI().nextInt(841) + 140;
    }

    public boolean h() {
        return !this.f() && this.c > this.b * 0.3f;
    }
}

