/*
 * Decompiled with CFR 0.152.
 */
public class wn
extends tg {
    private vk bq;

    public wn(ahb ahb2) {
        super(ahb2);
        this.a(0.6f, 0.8f);
        this.m().a(true);
        this.c.a(1, new uf(this));
        this.c.a(2, this.bp);
        this.bq = new vk(this, 0.6, ade.aP, true);
        this.c.a(3, this.bq);
        this.c.a(4, new tw(this, yz.class, 16.0f, 0.8, 1.33));
        this.c.a(5, new ug(this, 1.0, 10.0f, 5.0f));
        this.c.a(6, new uw(this, 1.33));
        this.c.a(7, new um(this, 0.3f));
        this.c.a(8, new uv(this));
        this.c.a(9, new ua(this, 0.8));
        this.c.a(10, new vc(this, 0.8));
        this.c.a(11, new un(this, yz.class, 10.0f));
        this.d.a(1, new vr(this, wg.class, 750, false));
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(18, (Object)0);
    }

    @Override
    public void bp() {
        if (this.k().a()) {
            double d2 = this.k().b();
            if (d2 == 0.6) {
                this.b(true);
                this.c(false);
            } else if (d2 == 1.33) {
                this.b(false);
                this.c(true);
            } else {
                this.b(false);
                this.c(false);
            }
        } else {
            this.b(false);
            this.c(false);
        }
    }

    @Override
    protected boolean v() {
        return !this.bZ() && this.aa > 2400;
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(10.0);
        this.a(yj.d).a(0.3f);
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("CatType", this.cg());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.s(dh2.f("CatType"));
    }

    @Override
    protected String t() {
        if (this.bZ()) {
            if (this.ce()) {
                return "mob.cat.purr";
            }
            if (this.Z.nextInt(4) == 0) {
                return "mob.cat.purreow";
            }
            return "mob.cat.meow";
        }
        return "";
    }

    @Override
    protected String aT() {
        return "mob.cat.hitt";
    }

    @Override
    protected String aU() {
        return "mob.cat.hitt";
    }

    @Override
    protected float bf() {
        return 0.4f;
    }

    @Override
    protected adb u() {
        return ade.aA;
    }

    @Override
    public boolean n(sa sa2) {
        return sa2.a(ro.a(this), 3.0f);
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        this.bp.a(false);
        return super.a(ro2, f2);
    }

    @Override
    protected void b(boolean bl2, int n2) {
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (this.bZ()) {
            if (this.e((sv)yz2) && !this.o.E && !this.c(add2)) {
                this.bp.a(!this.ca());
            }
        } else if (this.bq.f() && add2 != null && add2.b() == ade.aP && yz2.f(this) < 9.0) {
            if (!yz2.bE.d) {
                --add2.b;
            }
            if (add2.b <= 0) {
                yz2.bm.a(yz2.bm.c, null);
            }
            if (!this.o.E) {
                if (this.Z.nextInt(3) == 0) {
                    this.j(true);
                    this.s(1 + this.o.s.nextInt(3));
                    this.b(yz2.aB().toString());
                    this.i(true);
                    this.bp.a(true);
                    this.o.a((sa)this, (byte)7);
                } else {
                    this.i(false);
                    this.o.a((sa)this, (byte)6);
                }
            }
            return true;
        }
        return super.a(yz2);
    }

    public wn b(rx rx2) {
        wn wn2 = new wn(this.o);
        if (this.bZ()) {
            wn2.b(this.b());
            wn2.j(true);
            wn2.s(this.cg());
        }
        return wn2;
    }

    @Override
    public boolean c(add add2) {
        return add2 != null && add2.b() == ade.aP;
    }

    @Override
    public boolean a(wf wf2) {
        if (wf2 == this) {
            return false;
        }
        if (!this.bZ()) {
            return false;
        }
        if (!(wf2 instanceof wn)) {
            return false;
        }
        wn wn2 = (wn)wf2;
        if (!wn2.bZ()) {
            return false;
        }
        return this.ce() && wn2.ce();
    }

    public int cg() {
        return this.af.a(18);
    }

    public void s(int n2) {
        this.af.b(18, (byte)n2);
    }

    @Override
    public boolean by() {
        if (this.o.s.nextInt(3) == 0) {
            return false;
        }
        if (this.o.b(this.C) && this.o.a((sa)this, this.C).isEmpty() && !this.o.d(this.C)) {
            int n2 = qh.c(this.s);
            int n3 = qh.c(this.C.b);
            int n4 = qh.c(this.u);
            if (n3 < 63) {
                return false;
            }
            aji aji2 = this.o.a(n2, n3 - 1, n4);
            if (aji2 == ajn.c || aji2.o() == awt.j) {
                return true;
            }
        }
        return false;
    }

    @Override
    public String b_() {
        if (this.bH()) {
            return this.bG();
        }
        if (this.bZ()) {
            return dd.a("entity.Cat.name");
        }
        return super.b_();
    }

    @Override
    public sy a(sy sy2) {
        sy2 = super.a(sy2);
        if (this.o.s.nextInt(7) == 0) {
            for (int i2 = 0; i2 < 2; ++i2) {
                wn wn2 = new wn(this.o);
                wn2.b(this.s, this.t, this.u, this.y, 0.0f);
                wn2.c(-24000);
                this.o.d(wn2);
            }
        }
        return sy2;
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

