/*
 * Decompiled with CFR 0.152.
 */
public class wo
extends wf {
    private final ub bp;

    public wo(ahb ahb2) {
        super(ahb2);
        this.a(0.9f, 0.9f);
        this.m().a(true);
        this.c.a(0, new uf(this));
        this.c.a(1, new uz(this, 1.25));
        this.bp = new ub(this, 0.3f);
        this.c.a(2, this.bp);
        this.c.a(3, new ua(this, 1.0));
        this.c.a(4, new vk(this, 1.2, ade.bM, false));
        this.c.a(4, new vk(this, 1.2, ade.bF, false));
        this.c.a(5, new uh(this, 1.1));
        this.c.a(6, new vc(this, 1.0));
        this.c.a(7, new un(this, yz.class, 6.0f));
        this.c.a(8, new vb(this));
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(10.0);
        this.a(yj.d).a(0.25);
    }

    @Override
    protected void bn() {
        super.bn();
    }

    @Override
    public boolean bE() {
        add add2 = ((yz)this.l).be();
        return add2 != null && add2.b() == ade.bM;
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Saddle", this.bZ());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.i(dh2.n("Saddle"));
    }

    @Override
    protected String t() {
        return "mob.pig.say";
    }

    @Override
    protected String aT() {
        return "mob.pig.say";
    }

    @Override
    protected String aU() {
        return "mob.pig.death";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.pig.step", 0.15f, 1.0f);
    }

    @Override
    public boolean a(yz yz2) {
        if (!super.a(yz2)) {
            if (this.bZ() && !this.o.E && (this.l == null || this.l == yz2)) {
                yz2.a((sa)this);
                return true;
            }
            return false;
        }
        return true;
    }

    @Override
    protected adb u() {
        if (this.al()) {
            return ade.am;
        }
        return ade.al;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3 = this.Z.nextInt(3) + 1 + this.Z.nextInt(1 + n2);
        for (int i2 = 0; i2 < n3; ++i2) {
            if (this.al()) {
                this.a(ade.am, 1);
                continue;
            }
            this.a(ade.al, 1);
        }
        if (this.bZ()) {
            this.a(ade.av, 1);
        }
    }

    public boolean bZ() {
        return (this.af.a(16) & 1) != 0;
    }

    public void i(boolean bl2) {
        if (bl2) {
            this.af.b(16, (byte)1);
        } else {
            this.af.b(16, (byte)0);
        }
    }

    @Override
    public void a(xh xh2) {
        if (this.o.E) {
            return;
        }
        yh yh2 = new yh(this.o);
        yh2.c(0, new add(ade.B));
        yh2.b(this.s, this.t, this.u, this.y, this.z);
        this.o.d(yh2);
        this.B();
    }

    @Override
    protected void b(float f2) {
        super.b(f2);
        if (f2 > 5.0f && this.l instanceof yz) {
            ((yz)this.l).a(pc.u);
        }
    }

    public wo b(rx rx2) {
        return new wo(this.o);
    }

    @Override
    public boolean c(add add2) {
        return add2 != null && add2.b() == ade.bF;
    }

    public ub ca() {
        return this.bp;
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }
}

