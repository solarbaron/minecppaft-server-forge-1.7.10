/*
 * Decompiled with CFR 0.152.
 */
import java.util.Calendar;
import java.util.List;
import java.util.UUID;

public class yq
extends yg {
    protected static final th bp = new to("zombie.spawnReinforcements", 0.0, 0.0, 1.0).a("Spawn Reinforcements Chance");
    private static final UUID bq = UUID.fromString("B9766B59-9566-4402-BC1F-2EE2A276D836");
    private static final tj br = new tj(bq, "Baby speed boost", 0.5, 1);
    private final tz bs = new tz(this);
    private int bt;
    private boolean bu = false;
    private float bv = -1.0f;
    private float bw;

    public yq(ahb ahb2) {
        super(ahb2);
        this.m().b(true);
        this.c.a(0, new uf(this));
        this.c.a(2, new uq(this, yz.class, 1.0, false));
        this.c.a(4, new uq(this, yv.class, 1.0, true));
        this.c.a(5, new ut(this, 1.0));
        this.c.a(6, new us(this, 1.0, false));
        this.c.a(7, new vc(this, 1.0));
        this.c.a(8, new un(this, yz.class, 8.0f));
        this.c.a(8, new vb(this));
        this.d.a(1, new vn(this, true));
        this.d.a(2, new vo(this, yz.class, 0, true));
        this.d.a(2, new vo(this, yv.class, 0, false));
        this.a(0.6f, 1.8f);
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.b).a(40.0);
        this.a(yj.d).a(0.23f);
        this.a(yj.e).a(3.0);
        this.bc().b(bp).a(this.Z.nextDouble() * (double)0.1f);
    }

    @Override
    protected void c() {
        super.c();
        this.z().a(12, (Object)0);
        this.z().a(13, (Object)0);
        this.z().a(14, (Object)0);
    }

    @Override
    public int aV() {
        int n2 = super.aV() + 2;
        if (n2 > 20) {
            n2 = 20;
        }
        return n2;
    }

    @Override
    protected boolean bk() {
        return true;
    }

    public boolean bZ() {
        return this.bu;
    }

    public void a(boolean bl2) {
        if (this.bu != bl2) {
            this.bu = bl2;
            if (bl2) {
                this.c.a(1, this.bs);
            } else {
                this.c.a(this.bs);
            }
        }
    }

    @Override
    public boolean f() {
        return this.z().a(12) == 1;
    }

    @Override
    protected int e(yz yz2) {
        if (this.f()) {
            this.b = (int)((float)this.b * 2.5f);
        }
        return super.e(yz2);
    }

    public void i(boolean bl2) {
        this.z().b(12, (byte)(bl2 ? 1 : 0));
        if (this.o != null && !this.o.E) {
            ti ti2 = this.a(yj.d);
            ti2.b(br);
            if (bl2) {
                ti2.a(br);
            }
        }
        this.k(bl2);
    }

    public boolean cb() {
        return this.z().a(13) == 1;
    }

    public void j(boolean bl2) {
        this.z().b(13, (byte)(bl2 ? 1 : 0));
    }

    @Override
    public void e() {
        float f2;
        if (this.o.w() && !this.o.E && !this.f() && (f2 = this.d(1.0f)) > 0.5f && this.Z.nextFloat() * 30.0f < (f2 - 0.4f) * 2.0f && this.o.i(qh.c(this.s), qh.c(this.t), qh.c(this.u))) {
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
        if (this.am() && this.o() != null && this.m instanceof wg) {
            ((sw)this.m).m().a(this.m().e(), 1.5);
        }
        super.e();
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (super.a(ro2, f2)) {
            sv sv2 = this.o();
            if (sv2 == null && this.bT() instanceof sv) {
                sv2 = (sv)this.bT();
            }
            if (sv2 == null && ro2.j() instanceof sv) {
                sv2 = (sv)ro2.j();
            }
            if (sv2 != null && this.o.r == rd.d && (double)this.Z.nextFloat() < this.a(bp).e()) {
                int n2 = qh.c(this.s);
                int n3 = qh.c(this.t);
                int n4 = qh.c(this.u);
                yq yq2 = new yq(this.o);
                for (int i2 = 0; i2 < 50; ++i2) {
                    int n5;
                    int n6;
                    int n7 = n2 + qh.a(this.Z, 7, 40) * qh.a(this.Z, -1, 1);
                    if (!ahb.a(this.o, n7, (n6 = n3 + qh.a(this.Z, 7, 40) * qh.a(this.Z, -1, 1)) - 1, n5 = n4 + qh.a(this.Z, 7, 40) * qh.a(this.Z, -1, 1)) || this.o.k(n7, n6, n5) >= 10) continue;
                    yq2.b((double)n7, (double)n6, (double)n5);
                    if (!this.o.b(yq2.C) || !this.o.a((sa)yq2, yq2.C).isEmpty() || this.o.d(yq2.C)) continue;
                    this.o.d(yq2);
                    yq2.d(sv2);
                    yq2.a((sy)null);
                    this.a(bp).a(new tj("Zombie reinforcement caller charge", -0.05f, 0));
                    yq2.a(bp).a(new tj("Zombie reinforcement callee charge", -0.05f, 0));
                    break;
                }
            }
            return true;
        }
        return false;
    }

    @Override
    public void h() {
        if (!this.o.E && this.cc()) {
            int n2 = this.ce();
            this.bt -= n2;
            if (this.bt <= 0) {
                this.cd();
            }
        }
        super.h();
    }

    @Override
    public boolean n(sa sa2) {
        boolean bl2 = super.n(sa2);
        if (bl2) {
            int n2 = this.o.r.a();
            if (this.be() == null && this.al() && this.Z.nextFloat() < (float)n2 * 0.3f) {
                sa2.e(2 * n2);
            }
        }
        return bl2;
    }

    @Override
    protected String t() {
        return "mob.zombie.say";
    }

    @Override
    protected String aT() {
        return "mob.zombie.hurt";
    }

    @Override
    protected String aU() {
        return "mob.zombie.death";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        this.a("mob.zombie.step", 0.15f, 1.0f);
    }

    @Override
    protected adb u() {
        return ade.bh;
    }

    @Override
    public sz bd() {
        return sz.b;
    }

    @Override
    protected void n(int n2) {
        switch (this.Z.nextInt(3)) {
            case 0: {
                this.a(ade.j, 1);
                break;
            }
            case 1: {
                this.a(ade.bF, 1);
                break;
            }
            case 2: {
                this.a(ade.bG, 1);
            }
        }
    }

    @Override
    protected void bC() {
        super.bC();
        float f2 = this.Z.nextFloat();
        float f3 = this.o.r == rd.d ? 0.05f : 0.01f;
        if (f2 < f3) {
            int n2 = this.Z.nextInt(3);
            if (n2 == 0) {
                this.c(0, new add(ade.l));
            } else {
                this.c(0, new add(ade.a));
            }
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.f()) {
            dh2.a("IsBaby", true);
        }
        if (this.cb()) {
            dh2.a("IsVillager", true);
        }
        dh2.a("ConversionTime", this.cc() ? this.bt : -1);
        dh2.a("CanBreakDoors", this.bZ());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.n("IsBaby")) {
            this.i(true);
        }
        if (dh2.n("IsVillager")) {
            this.j(true);
        }
        if (dh2.b("ConversionTime", 99) && dh2.f("ConversionTime") > -1) {
            this.a(dh2.f("ConversionTime"));
        }
        this.a(dh2.n("CanBreakDoors"));
    }

    @Override
    public void a(sv sv2) {
        super.a(sv2);
        if ((this.o.r == rd.c || this.o.r == rd.d) && sv2 instanceof yv) {
            if (this.o.r != rd.d && this.Z.nextBoolean()) {
                return;
            }
            yq yq2 = new yq(this.o);
            yq2.k(sv2);
            this.o.e(sv2);
            yq2.a((sy)null);
            yq2.j(true);
            if (sv2.f()) {
                yq2.i(true);
            }
            this.o.d(yq2);
            this.o.a(null, 1016, (int)this.s, (int)this.t, (int)this.u, 0);
        }
    }

    @Override
    public sy a(sy sy2) {
        Object object;
        sy2 = super.a(sy2);
        float f2 = this.o.b(this.s, this.t, this.u);
        this.h(this.Z.nextFloat() < 0.55f * f2);
        if (sy2 == null) {
            sy2 = new ys(this, this.o.s.nextFloat() < 0.05f, this.o.s.nextFloat() < 0.05f, null);
        }
        if (sy2 instanceof ys) {
            object = (ys)sy2;
            if (((ys)object).b) {
                this.j(true);
            }
            if (((ys)object).a) {
                this.i(true);
                if ((double)this.o.s.nextFloat() < 0.05) {
                    List list = this.o.a(wg.class, this.C.b(5.0, 3.0, 5.0), sj.b);
                    if (!list.isEmpty()) {
                        wg wg2 = (wg)list.get(0);
                        wg2.i(true);
                        this.a((sa)wg2);
                    }
                } else if ((double)this.o.s.nextFloat() < 0.05) {
                    wg wg3 = new wg(this.o);
                    wg3.b(this.s, this.t, this.u, this.y, 0.0f);
                    wg3.a((sy)null);
                    wg3.i(true);
                    this.o.d(wg3);
                    this.a((sa)wg3);
                }
            }
        }
        this.a(this.Z.nextFloat() < f2 * 0.1f);
        this.bC();
        this.bD();
        if (this.q(4) == null && ((Calendar)(object = this.o.V())).get(2) + 1 == 10 && ((Calendar)object).get(5) == 31 && this.Z.nextFloat() < 0.25f) {
            this.c(4, new add(this.Z.nextFloat() < 0.1f ? ajn.aP : ajn.aK));
            this.e[4] = 0.0f;
        }
        this.a(yj.c).a(new tj("Random spawn bonus", this.Z.nextDouble() * (double)0.05f, 0));
        double d2 = this.Z.nextDouble() * 1.5 * (double)this.o.b(this.s, this.t, this.u);
        if (d2 > 1.0) {
            this.a(yj.b).a(new tj("Random zombie-spawn bonus", d2, 2));
        }
        if (this.Z.nextFloat() < f2 * 0.05f) {
            this.a(bp).a(new tj("Leader zombie bonus", this.Z.nextDouble() * 0.25 + 0.5, 0));
            this.a(yj.a).a(new tj("Leader zombie bonus", this.Z.nextDouble() * 3.0 + 1.0, 2));
            this.a(true);
        }
        return sy2;
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bF();
        if (add2 != null && add2.b() == ade.ao && add2.k() == 0 && this.cb() && this.a(rv.t)) {
            if (!yz2.bE.d) {
                --add2.b;
            }
            if (add2.b <= 0) {
                yz2.bm.a(yz2.bm.c, null);
            }
            if (!this.o.E) {
                this.a(this.Z.nextInt(2401) + 3600);
            }
            return true;
        }
        return false;
    }

    protected void a(int n2) {
        this.bt = n2;
        this.z().b(14, (byte)1);
        this.m(rv.t.H);
        this.c(new rw(rv.g.H, n2, Math.min(this.o.r.a() - 1, 0)));
        this.o.a((sa)this, (byte)16);
    }

    @Override
    protected boolean v() {
        return !this.cc();
    }

    public boolean cc() {
        return this.z().a(14) == 1;
    }

    protected void cd() {
        yv yv2 = new yv(this.o);
        yv2.k(this);
        yv2.a((sy)null);
        yv2.cd();
        if (this.f()) {
            yv2.c(-24000);
        }
        this.o.e(this);
        this.o.d(yv2);
        yv2.c(new rw(rv.k.H, 200, 0));
        this.o.a(null, 1017, (int)this.s, (int)this.t, (int)this.u, 0);
    }

    protected int ce() {
        int n2 = 1;
        if (this.Z.nextFloat() < 0.01f) {
            int n3 = 0;
            for (int i2 = (int)this.s - 4; i2 < (int)this.s + 4 && n3 < 14; ++i2) {
                for (int i3 = (int)this.t - 4; i3 < (int)this.t + 4 && n3 < 14; ++i3) {
                    for (int i4 = (int)this.u - 4; i4 < (int)this.u + 4 && n3 < 14; ++i4) {
                        aji aji2 = this.o.a(i2, i3, i4);
                        if (aji2 != ajn.aY && aji2 != ajn.C) continue;
                        if (this.Z.nextFloat() < 0.3f) {
                            ++n2;
                        }
                        ++n3;
                    }
                }
            }
        }
        return n2;
    }

    public void k(boolean bl2) {
        this.a(bl2 ? 0.5f : 1.0f);
    }

    @Override
    protected final void a(float f2, float f3) {
        boolean bl2 = this.bv > 0.0f && this.bw > 0.0f;
        this.bv = f2;
        this.bw = f3;
        if (!bl2) {
            this.a(1.0f);
        }
    }

    protected final void a(float f2) {
        super.a(this.bv * f2, this.bw * f2);
    }
}

