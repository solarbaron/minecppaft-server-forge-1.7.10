/*
 * Decompiled with CFR 0.152.
 */
public class wg
extends wf {
    public float bp;
    public float bq;
    public float br;
    public float bs;
    public float bt = 1.0f;
    public int bu;
    public boolean bv;

    public wg(ahb ahb2) {
        super(ahb2);
        this.a(0.3f, 0.7f);
        this.bu = this.Z.nextInt(6000) + 6000;
        this.c.a(0, new uf(this));
        this.c.a(1, new uz(this, 1.4));
        this.c.a(2, new ua(this, 1.0));
        this.c.a(3, new vk(this, 1.0, ade.N, false));
        this.c.a(4, new uh(this, 1.1));
        this.c.a(5, new vc(this, 1.0));
        this.c.a(6, new un(this, yz.class, 6.0f));
        this.c.a(7, new vb(this));
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(4.0);
        this.a(yj.d).a(0.25);
    }

    @Override
    public void e() {
        super.e();
        this.bs = this.bp;
        this.br = this.bq;
        this.bq = (float)((double)this.bq + (double)(this.D ? -1 : 4) * 0.3);
        if (this.bq < 0.0f) {
            this.bq = 0.0f;
        }
        if (this.bq > 1.0f) {
            this.bq = 1.0f;
        }
        if (!this.D && this.bt < 1.0f) {
            this.bt = 1.0f;
        }
        this.bt = (float)((double)this.bt * 0.9);
        if (!this.D && this.w < 0.0) {
            this.w *= 0.6;
        }
        this.bp += this.bt * 2.0f;
        if (!(this.o.E || this.f() || this.bZ() || --this.bu > 0)) {
            this.a("mob.chicken.plop", 1.0f, (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f);
            this.a(ade.aK, 1);
            this.bu = this.Z.nextInt(6000) + 6000;
        }
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    protected String t() {
        return "mob.chicken.say";
    }

    @Override
    protected String aT() {
        return "mob.chicken.hurt";
    }

    @Override
    protected String aU() {
        return "mob.chicken.hurt";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.chicken.step", 0.15f, 1.0f);
    }

    @Override
    protected adb u() {
        return ade.G;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3 = this.Z.nextInt(3) + this.Z.nextInt(1 + n2);
        for (int i2 = 0; i2 < n3; ++i2) {
            this.a(ade.G, 1);
        }
        if (this.al()) {
            this.a(ade.bg, 1);
        } else {
            this.a(ade.bf, 1);
        }
    }

    public wg b(rx rx2) {
        return new wg(this.o);
    }

    @Override
    public boolean c(add add2) {
        return add2 != null && add2.b() instanceof adw;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.bv = dh2.n("IsChickenJockey");
    }

    @Override
    protected int e(yz yz2) {
        if (this.bZ()) {
            return 10;
        }
        return super.e(yz2);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("IsChickenJockey", this.bv);
    }

    @Override
    protected boolean v() {
        return this.bZ() && this.l == null;
    }

    @Override
    public void ac() {
        super.ac();
        float f2 = qh.a(this.aM * (float)Math.PI / 180.0f);
        float f3 = qh.b(this.aM * (float)Math.PI / 180.0f);
        float f4 = 0.1f;
        float f5 = 0.0f;
        this.l.b(this.s + (double)(f4 * f2), this.t + (double)(this.N * 0.5f) + this.l.ad() + (double)f5, this.u - (double)(f4 * f3));
        if (this.l instanceof sv) {
            ((sv)this.l).aM = this.aM;
        }
    }

    public boolean bZ() {
        return this.bv;
    }

    public void i(boolean bl2) {
        this.bv = bl2;
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

