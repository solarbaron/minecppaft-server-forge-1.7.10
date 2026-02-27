/*
 * Decompiled with CFR 0.152.
 */
import java.util.Calendar;

public class yl
extends yg
implements yi {
    private vd bp = new vd(this, 1.0, 20, 60, 15.0f);
    private uq bq = new uq(this, yz.class, 1.2, false);

    public yl(ahb ahb2) {
        super(ahb2);
        this.c.a(1, new uf(this));
        this.c.a(2, new vf(this));
        this.c.a(3, new ue(this, 1.0));
        this.c.a(5, new vc(this, 1.0));
        this.c.a(6, new un(this, yz.class, 8.0f));
        this.c.a(6, new vb(this));
        this.d.a(1, new vn(this, false));
        this.d.a(2, new vo(this, yz.class, 0, true));
        if (ahb2 != null && !ahb2.E) {
            this.bZ();
        }
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.d).a(0.25);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(13, new Byte(0));
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected String t() {
        return "mob.skeleton.say";
    }

    @Override
    protected String aT() {
        return "mob.skeleton.hurt";
    }

    @Override
    protected String aU() {
        return "mob.skeleton.death";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.skeleton.step", 0.15f, 1.0f);
    }

    @Override
    public boolean n(sa sa2) {
        if (super.n(sa2)) {
            if (this.cb() == 1 && sa2 instanceof sv) {
                ((sv)sa2).c(new rw(rv.v.H, 200));
            }
            return true;
        }
        return false;
    }

    @Override
    public sz bd() {
        return sz.b;
    }

    @Override
    public void e() {
        float f2;
        if (this.o.w() && !this.o.E && (f2 = this.d(1.0f)) > 0.5f && this.Z.nextFloat() * 30.0f < (f2 - 0.4f) * 2.0f && this.o.i(qh.c(this.s), qh.c(this.t), qh.c(this.u))) {
            boolean bl2 = true;
            add add2 = this.q(4);
            if (add2 != null) {
                if (add2.g()) {
                    add2.b(add2.j() + this.Z.nextInt(2));
                    if (add2.j() >= add2.l()) {
                        this.a(add2);
                        this.c(4, null);
                    }
                }
                bl2 = false;
            }
            if (bl2) {
                this.e(8);
            }
        }
        if (this.o.E && this.cb() == 1) {
            this.a(0.72f, 2.34f);
        }
        super.e();
    }

    @Override
    public void ab() {
        super.ab();
        if (this.m instanceof td) {
            td td2 = (td)this.m;
            this.aM = td2.aM;
        }
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        if (ro2.i() instanceof zc && ro2.j() instanceof yz) {
            yz yz2 = (yz)ro2.j();
            double d2 = yz2.s - this.s;
            double d3 = yz2.u - this.u;
            if (d2 * d2 + d3 * d3 >= 2500.0) {
                yz2.a(pc.v);
            }
        }
    }

    @Override
    protected adb u() {
        return ade.g;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3;
        int n4;
        if (this.cb() == 1) {
            n4 = this.Z.nextInt(3 + n2) - 1;
            for (n3 = 0; n3 < n4; ++n3) {
                this.a(ade.h, 1);
            }
        } else {
            n4 = this.Z.nextInt(3 + n2);
            for (n3 = 0; n3 < n4; ++n3) {
                this.a(ade.g, 1);
            }
        }
        n4 = this.Z.nextInt(3 + n2);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(ade.aS, 1);
        }
    }

    @Override
    protected void n(int n2) {
        if (this.cb() == 1) {
            this.a(new add(ade.bL, 1, 1), 0.0f);
        }
    }

    @Override
    protected void bC() {
        super.bC();
        this.c(0, new add(ade.f));
    }

    @Override
    public sy a(sy sy2) {
        Calendar calendar;
        sy2 = super.a(sy2);
        if (this.o.t instanceof aqp && this.aI().nextInt(5) > 0) {
            this.c.a(4, this.bq);
            this.a(1);
            this.c(0, new add(ade.q));
            this.a(yj.e).a(4.0);
        } else {
            this.c.a(4, this.bp);
            this.bC();
            this.bD();
        }
        this.h(this.Z.nextFloat() < 0.55f * this.o.b(this.s, this.t, this.u));
        if (this.q(4) == null && (calendar = this.o.V()).get(2) + 1 == 10 && calendar.get(5) == 31 && this.Z.nextFloat() < 0.25f) {
            this.c(4, new add(this.Z.nextFloat() < 0.1f ? ajn.aP : ajn.aK));
            this.e[4] = 0.0f;
        }
        return sy2;
    }

    public void bZ() {
        this.c.a(this.bq);
        this.c.a(this.bp);
        add add2 = this.be();
        if (add2 != null && add2.b() == ade.f) {
            this.c.a(4, this.bp);
        } else {
            this.c.a(4, this.bq);
        }
    }

    @Override
    public void a(sv sv2, float f2) {
        zc zc2 = new zc(this.o, this, sv2, 1.6f, 14 - this.o.r.a() * 4);
        int n2 = afv.a(aft.v.B, this.be());
        int n3 = afv.a(aft.w.B, this.be());
        zc2.b((double)(f2 * 2.0f) + (this.Z.nextGaussian() * 0.25 + (double)((float)this.o.r.a() * 0.11f)));
        if (n2 > 0) {
            zc2.b(zc2.e() + (double)n2 * 0.5 + 0.5);
        }
        if (n3 > 0) {
            zc2.a(n3);
        }
        if (afv.a(aft.x.B, this.be()) > 0 || this.cb() == 1) {
            zc2.e(100);
        }
        this.a("random.bow", 1.0f, 1.0f / (this.aI().nextFloat() * 0.4f + 0.8f));
        this.o.d(zc2);
    }

    public int cb() {
        return this.af.a(13);
    }

    public void a(int n2) {
        this.af.b(13, (byte)n2);
        boolean bl2 = this.ae = n2 == 1;
        if (n2 == 1) {
            this.a(0.72f, 2.34f);
        } else {
            this.a(0.6f, 1.8f);
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("SkeletonType", 99)) {
            byte by2 = dh2.d("SkeletonType");
            this.a(by2);
        }
        this.bZ();
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("SkeletonType", (byte)this.cb());
    }

    @Override
    public void c(int n2, add add2) {
        super.c(n2, add2);
        if (!this.o.E && n2 == 0) {
            this.bZ();
        }
    }

    @Override
    public double ad() {
        return super.ad() - 0.5;
    }
}

