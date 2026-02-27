/*
 * Decompiled with CFR 0.152.
 */
public class wv
extends tg {
    private float bq;
    private float br;
    private boolean bs;
    private boolean bt;
    private float bu;
    private float bv;

    public wv(ahb ahb2) {
        super(ahb2);
        this.a(0.6f, 0.8f);
        this.m().a(true);
        this.c.a(1, new uf(this));
        this.c.a(2, this.bp);
        this.c.a(3, new um(this, 0.4f));
        this.c.a(4, new uq(this, 1.0, true));
        this.c.a(5, new ug(this, 1.0, 10.0f, 2.0f));
        this.c.a(6, new ua(this, 1.0));
        this.c.a(7, new vc(this, 1.0));
        this.c.a(8, new ty(this, 8.0f));
        this.c.a(9, new un(this, yz.class, 8.0f));
        this.c.a(9, new vb(this));
        this.d.a(1, new vs(this));
        this.d.a(2, new vt(this));
        this.d.a(3, new vn(this, true));
        this.d.a(4, new vr(this, wp.class, 200, false));
        this.j(false);
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.d).a(0.3f);
        if (this.bZ()) {
            this.a(yj.a).a(20.0);
        } else {
            this.a(yj.a).a(8.0);
        }
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    public void d(sv sv2) {
        super.d(sv2);
        if (sv2 == null) {
            this.l(false);
        } else if (!this.bZ()) {
            this.l(true);
        }
    }

    @Override
    protected void bp() {
        this.af.b(18, Float.valueOf(this.aS()));
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(18, new Float(this.aS()));
        this.af.a(19, new Byte(0));
        this.af.a(20, new Byte((byte)aka.b(1)));
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.wolf.step", 0.15f, 1.0f);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Angry", this.ci());
        dh2.a("CollarColor", (byte)this.cj());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.l(dh2.n("Angry"));
        if (dh2.b("CollarColor", 99)) {
            this.s(dh2.d("CollarColor"));
        }
    }

    @Override
    protected String t() {
        if (this.ci()) {
            return "mob.wolf.growl";
        }
        if (this.Z.nextInt(3) == 0) {
            if (this.bZ() && this.af.d(18) < 10.0f) {
                return "mob.wolf.whine";
            }
            return "mob.wolf.panting";
        }
        return "mob.wolf.bark";
    }

    @Override
    protected String aT() {
        return "mob.wolf.hurt";
    }

    @Override
    protected String aU() {
        return "mob.wolf.death";
    }

    @Override
    protected float bf() {
        return 0.4f;
    }

    @Override
    protected adb u() {
        return adb.d(-1);
    }

    @Override
    public void e() {
        super.e();
        if (!this.o.E && this.bs && !this.bt && !this.bS() && this.D) {
            this.bt = true;
            this.bu = 0.0f;
            this.bv = 0.0f;
            this.o.a((sa)this, (byte)8);
        }
    }

    @Override
    public void h() {
        super.h();
        this.br = this.bq;
        this.bq = this.ck() ? (this.bq += (1.0f - this.bq) * 0.4f) : (this.bq += (0.0f - this.bq) * 0.4f);
        if (this.ck()) {
            this.g = 10;
        }
        if (this.L()) {
            this.bs = true;
            this.bt = false;
            this.bu = 0.0f;
            this.bv = 0.0f;
        } else if ((this.bs || this.bt) && this.bt) {
            if (this.bu == 0.0f) {
                this.a("mob.wolf.shake", this.bf(), (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f);
            }
            this.bv = this.bu;
            this.bu += 0.05f;
            if (this.bv >= 2.0f) {
                this.bs = false;
                this.bt = false;
                this.bv = 0.0f;
                this.bu = 0.0f;
            }
            if (this.bu > 0.4f) {
                float f2 = (float)this.C.b;
                int n2 = (int)(qh.a((this.bu - 0.4f) * (float)Math.PI) * 7.0f);
                for (int i2 = 0; i2 < n2; ++i2) {
                    float f3 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M * 0.5f;
                    float f4 = (this.Z.nextFloat() * 2.0f - 1.0f) * this.M * 0.5f;
                    this.o.a("splash", this.s + (double)f3, (double)(f2 + 0.8f), this.u + (double)f4, this.v, this.w, this.x);
                }
            }
        }
    }

    @Override
    public float g() {
        return this.N * 0.8f;
    }

    @Override
    public int x() {
        if (this.ca()) {
            return 20;
        }
        return super.x();
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        sa sa2 = ro2.j();
        this.bp.a(false);
        if (sa2 != null && !(sa2 instanceof yz) && !(sa2 instanceof zc)) {
            f2 = (f2 + 1.0f) / 2.0f;
        }
        return super.a(ro2, f2);
    }

    @Override
    public boolean n(sa sa2) {
        int n2 = this.bZ() ? 4 : 2;
        return sa2.a(ro.a(this), (float)n2);
    }

    @Override
    public void j(boolean bl2) {
        super.j(bl2);
        if (bl2) {
            this.a(yj.a).a(20.0);
        } else {
            this.a(yj.a).a(8.0);
        }
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (this.bZ()) {
            if (add2 != null) {
                int n2;
                if (add2.b() instanceof acx) {
                    acx acx2 = (acx)add2.b();
                    if (acx2.i() && this.af.d(18) < 20.0f) {
                        if (!yz2.bE.d) {
                            --add2.b;
                        }
                        this.f((float)acx2.g(add2));
                        if (add2.b <= 0) {
                            yz2.bm.a(yz2.bm.c, null);
                        }
                        return true;
                    }
                } else if (add2.b() == ade.aR && (n2 = aka.b(add2.k())) != this.cj()) {
                    this.s(n2);
                    if (!yz2.bE.d && --add2.b <= 0) {
                        yz2.bm.a(yz2.bm.c, null);
                    }
                    return true;
                }
            }
            if (this.e((sv)yz2) && !this.o.E && !this.c(add2)) {
                this.bp.a(!this.ca());
                this.bc = false;
                this.a((ayf)null);
                this.b((sa)null);
                this.d((sv)null);
            }
        } else if (add2 != null && add2.b() == ade.aS && !this.ci()) {
            if (!yz2.bE.d) {
                --add2.b;
            }
            if (add2.b <= 0) {
                yz2.bm.a(yz2.bm.c, null);
            }
            if (!this.o.E) {
                if (this.Z.nextInt(3) == 0) {
                    this.j(true);
                    this.a((ayf)null);
                    this.d((sv)null);
                    this.bp.a(true);
                    this.g(20.0f);
                    this.b(yz2.aB().toString());
                    this.i(true);
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

    @Override
    public boolean c(add add2) {
        if (add2 == null) {
            return false;
        }
        if (!(add2.b() instanceof acx)) {
            return false;
        }
        return ((acx)add2.b()).i();
    }

    @Override
    public int bB() {
        return 8;
    }

    public boolean ci() {
        return (this.af.a(16) & 2) != 0;
    }

    public void l(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 2));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFD));
        }
    }

    public int cj() {
        return this.af.a(20) & 0xF;
    }

    public void s(int n2) {
        this.af.b(20, (byte)(n2 & 0xF));
    }

    public wv b(rx rx2) {
        wv wv2 = new wv(this.o);
        String string = this.b();
        if (string != null && string.trim().length() > 0) {
            wv2.b(string);
            wv2.j(true);
        }
        return wv2;
    }

    public void m(boolean bl2) {
        if (bl2) {
            this.af.b(19, (byte)1);
        } else {
            this.af.b(19, (byte)0);
        }
    }

    @Override
    public boolean a(wf wf2) {
        if (wf2 == this) {
            return false;
        }
        if (!this.bZ()) {
            return false;
        }
        if (!(wf2 instanceof wv)) {
            return false;
        }
        wv wv2 = (wv)wf2;
        if (!wv2.bZ()) {
            return false;
        }
        if (wv2.ca()) {
            return false;
        }
        return this.ce() && wv2.ce();
    }

    public boolean ck() {
        return this.af.a(19) == 1;
    }

    @Override
    protected boolean v() {
        return !this.bZ() && this.aa > 2400;
    }

    @Override
    public boolean a(sv sv2, sv sv3) {
        wv wv2;
        if (sv2 instanceof xz || sv2 instanceof yd) {
            return false;
        }
        if (sv2 instanceof wv && (wv2 = (wv)sv2).bZ() && wv2.cb() == sv3) {
            return false;
        }
        if (sv2 instanceof yz && sv3 instanceof yz && !((yz)sv3).a((yz)sv2)) {
            return false;
        }
        return !(sv2 instanceof wi) || !((wi)sv2).cc();
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

