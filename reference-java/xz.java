/*
 * Decompiled with CFR 0.152.
 */
public class xz
extends yg {
    private int bp;
    private int bq;
    private int br = 30;
    private int bs = 3;

    public xz(ahb ahb2) {
        super(ahb2);
        this.c.a(1, new uf(this));
        this.c.a(2, new vi(this));
        this.c.a(3, new tw(this, wn.class, 6.0f, 1.0, 1.2));
        this.c.a(4, new uq(this, 1.0, false));
        this.c.a(5, new vc(this, 0.8));
        this.c.a(6, new un(this, yz.class, 8.0f));
        this.c.a(6, new vb(this));
        this.d.a(1, new vo(this, yz.class, 0, true));
        this.d.a(2, new vn(this, false));
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.d).a(0.25);
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    public int ax() {
        if (this.o() == null) {
            return 3;
        }
        return 3 + (int)(this.aS() - 1.0f);
    }

    @Override
    protected void b(float f2) {
        super.b(f2);
        this.bq = (int)((float)this.bq + f2 * 1.5f);
        if (this.bq > this.br - 5) {
            this.bq = this.br - 5;
        }
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)-1);
        this.af.a(17, (Object)0);
        this.af.a(18, (Object)0);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.af.a(17) == 1) {
            dh2.a("powered", true);
        }
        dh2.a("Fuse", (short)this.br);
        dh2.a("ExplosionRadius", (byte)this.bs);
        dh2.a("ignited", this.cc());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.af.b(17, (byte)(dh2.n("powered") ? 1 : 0));
        if (dh2.b("Fuse", 99)) {
            this.br = dh2.e("Fuse");
        }
        if (dh2.b("ExplosionRadius", 99)) {
            this.bs = dh2.d("ExplosionRadius");
        }
        if (dh2.n("ignited")) {
            this.cd();
        }
    }

    @Override
    public void h() {
        if (this.Z()) {
            int n2;
            this.bp = this.bq;
            if (this.cc()) {
                this.a(1);
            }
            if ((n2 = this.cb()) > 0 && this.bq == 0) {
                this.a("creeper.primed", 1.0f, 0.5f);
            }
            this.bq += n2;
            if (this.bq < 0) {
                this.bq = 0;
            }
            if (this.bq >= this.br) {
                this.bq = this.br;
                this.ce();
            }
        }
        super.h();
    }

    @Override
    protected String aT() {
        return "mob.creeper.say";
    }

    @Override
    protected String aU() {
        return "mob.creeper.death";
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        if (ro2.j() instanceof yl) {
            int n2 = adb.b(ade.cd);
            int n3 = adb.b(ade.co);
            int n4 = n2 + this.Z.nextInt(n3 - n2 + 1);
            this.a(adb.d(n4), 1);
        }
    }

    @Override
    public boolean n(sa sa2) {
        return true;
    }

    public boolean bZ() {
        return this.af.a(17) == 1;
    }

    @Override
    protected adb u() {
        return ade.H;
    }

    public int cb() {
        return this.af.a(16);
    }

    public void a(int n2) {
        this.af.b(16, (byte)n2);
    }

    @Override
    public void a(xh xh2) {
        super.a(xh2);
        this.af.b(17, (byte)1);
    }

    @Override
    protected boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.d) {
            this.o.a(this.s + 0.5, this.t + 0.5, this.u + 0.5, "fire.ignite", 1.0f, this.Z.nextFloat() * 0.4f + 0.8f);
            yz2.ba();
            if (!this.o.E) {
                this.cd();
                add2.a(1, (sv)yz2);
                return true;
            }
        }
        return super.a(yz2);
    }

    private void ce() {
        if (!this.o.E) {
            boolean bl2 = this.o.O().b("mobGriefing");
            if (this.bZ()) {
                this.o.a(this, this.s, this.t, this.u, (float)(this.bs * 2), bl2);
            } else {
                this.o.a(this, this.s, this.t, this.u, (float)this.bs, bl2);
            }
            this.B();
        }
    }

    public boolean cc() {
        return this.af.a(18) != 0;
    }

    public void cd() {
        this.af.b(18, (byte)1);
    }
}

