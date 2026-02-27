/*
 * Decompiled with CFR 0.152.
 */
public class wt
extends wl {
    private int bq;
    vz bp;
    private int br;
    private int bs;

    public wt(ahb ahb2) {
        super(ahb2);
        this.a(1.4f, 2.9f);
        this.m().a(true);
        this.c.a(1, new uq(this, 1.0, true));
        this.c.a(2, new uu(this, 0.9, 32.0f));
        this.c.a(3, new us(this, 0.6, true));
        this.c.a(4, new ut(this, 1.0));
        this.c.a(5, new ux(this));
        this.c.a(6, new vc(this, 0.6));
        this.c.a(7, new un(this, yz.class, 6.0f));
        this.c.a(8, new vb(this));
        this.d.a(1, new vm(this));
        this.d.a(2, new vn(this, false));
        this.d.a(3, new vo(this, sw.class, 0, false, true, yb.a));
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void bp() {
        if (--this.bq <= 0) {
            this.bq = 70 + this.Z.nextInt(50);
            this.bp = this.o.A.a(qh.c(this.s), qh.c(this.t), qh.c(this.u), 32);
            if (this.bp == null) {
                this.bX();
            } else {
                r r2 = this.bp.a();
                this.a(r2.a, r2.b, r2.c, (int)((float)this.bp.b() * 0.6f));
            }
        }
        super.bp();
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(100.0);
        this.a(yj.d).a(0.25);
    }

    @Override
    protected int j(int n2) {
        return n2;
    }

    @Override
    protected void o(sa sa2) {
        if (sa2 instanceof yb && this.aI().nextInt(20) == 0) {
            this.d((sv)sa2);
        }
        super.o(sa2);
    }

    @Override
    public void e() {
        int n2;
        int n3;
        int n4;
        aji aji2;
        super.e();
        if (this.br > 0) {
            --this.br;
        }
        if (this.bs > 0) {
            --this.bs;
        }
        if (this.v * this.v + this.x * this.x > 2.500000277905201E-7 && this.Z.nextInt(5) == 0 && (aji2 = this.o.a(n4 = qh.c(this.s), n3 = qh.c(this.t - (double)0.2f - (double)this.L), n2 = qh.c(this.u))).o() != awt.a) {
            this.o.a("blockcrack_" + aji.b(aji2) + "_" + this.o.e(n4, n3, n2), this.s + ((double)this.Z.nextFloat() - 0.5) * (double)this.M, this.C.b + 0.1, this.u + ((double)this.Z.nextFloat() - 0.5) * (double)this.M, 4.0 * ((double)this.Z.nextFloat() - 0.5), 0.5, ((double)this.Z.nextFloat() - 0.5) * 4.0);
        }
    }

    @Override
    public boolean a(Class clazz) {
        if (this.cc() && yz.class.isAssignableFrom(clazz)) {
            return false;
        }
        return super.a(clazz);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("PlayerCreated", this.cc());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.i(dh2.n("PlayerCreated"));
    }

    @Override
    public boolean n(sa sa2) {
        this.br = 10;
        this.o.a((sa)this, (byte)4);
        boolean bl2 = sa2.a(ro.a(this), (float)(7 + this.Z.nextInt(15)));
        if (bl2) {
            sa2.w += (double)0.4f;
        }
        this.a("mob.irongolem.throw", 1.0f, 1.0f);
        return bl2;
    }

    public vz bZ() {
        return this.bp;
    }

    public void a(boolean bl2) {
        this.bs = bl2 ? 400 : 0;
        this.o.a((sa)this, (byte)11);
    }

    @Override
    protected String aT() {
        return "mob.irongolem.hit";
    }

    @Override
    protected String aU() {
        return "mob.irongolem.death";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.irongolem.walk", 1.0f, 1.0f);
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3;
        int n4 = this.Z.nextInt(3);
        for (n3 = 0; n3 < n4; ++n3) {
            this.a(adb.a(ajn.O), 1, 0.0f);
        }
        n3 = 3 + this.Z.nextInt(3);
        for (int i2 = 0; i2 < n3; ++i2) {
            this.a(ade.j, 1);
        }
    }

    public int cb() {
        return this.bs;
    }

    public boolean cc() {
        return (this.af.a(16) & 1) != 0;
    }

    public void i(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 1));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFE));
        }
    }

    @Override
    public void a(ro ro2) {
        if (!this.cc() && this.aR != null && this.bp != null) {
            this.bp.a(this.aR.b_(), -5);
        }
        super.a(ro2);
    }
}

