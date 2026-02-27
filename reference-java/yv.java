/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class yv
extends rx
implements agm,
yu {
    private int bq;
    private boolean br;
    private boolean bs;
    vz bp;
    private yz bt;
    private ago bu;
    private int bv;
    private boolean bw;
    private int bx;
    private String by;
    private boolean bz;
    private float bA;
    private static final Map bB = new HashMap();
    private static final Map bC = new HashMap();

    public yv(ahb ahb2) {
        this(ahb2, 0);
    }

    public yv(ahb ahb2, int n2) {
        super(ahb2);
        this.s(n2);
        this.a(0.6f, 1.8f);
        this.m().b(true);
        this.m().a(true);
        this.c.a(0, new uf(this));
        this.c.a(1, new tw(this, yq.class, 8.0f, 0.6, 0.6));
        this.c.a(1, new vl(this));
        this.c.a(1, new uo(this));
        this.c.a(2, new ur(this));
        this.c.a(3, new ve(this));
        this.c.a(4, new uy(this, true));
        this.c.a(5, new ut(this, 0.6));
        this.c.a(6, new up(this));
        this.c.a(7, new vj(this));
        this.c.a(8, new va(this, 0.32));
        this.c.a(9, new ul(this, yz.class, 3.0f, 1.0f));
        this.c.a(9, new ul(this, yv.class, 5.0f, 0.02f));
        this.c.a(9, new vc(this, 0.6));
        this.c.a(10, new un(this, sw.class, 8.0f));
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.d).a(0.5);
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void bp() {
        if (--this.bq <= 0) {
            this.o.A.a(qh.c(this.s), qh.c(this.t), qh.c(this.u));
            this.bq = 70 + this.Z.nextInt(50);
            this.bp = this.o.A.a(qh.c(this.s), qh.c(this.t), qh.c(this.u), 32);
            if (this.bp == null) {
                this.bX();
            } else {
                r r2 = this.bp.a();
                this.a(r2.a, r2.b, r2.c, (int)((float)this.bp.b() * 0.6f));
                if (this.bz) {
                    this.bz = false;
                    this.bp.b(5);
                }
            }
        }
        if (!this.cc() && this.bv > 0) {
            --this.bv;
            if (this.bv <= 0) {
                if (this.bw) {
                    if (this.bu.size() > 1) {
                        for (agn agn2 : this.bu) {
                            if (!agn2.g()) continue;
                            agn2.a(this.Z.nextInt(6) + this.Z.nextInt(6) + 2);
                        }
                    }
                    this.t(1);
                    this.bw = false;
                    if (this.bp != null && this.by != null) {
                        this.o.a((sa)this, (byte)14);
                        this.bp.a(this.by, 1);
                    }
                }
                this.c(new rw(rv.l.H, 200, 0));
            }
        }
        super.bp();
    }

    @Override
    public boolean a(yz yz2) {
        boolean bl2;
        add add2 = yz2.bm.h();
        boolean bl3 = bl2 = add2 != null && add2.b() == ade.bx;
        if (!bl2 && this.Z() && !this.cc() && !this.f()) {
            if (!this.o.E) {
                this.a_(yz2);
                yz2.a(this, this.bG());
            }
            return true;
        }
        return super.a(yz2);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Profession", this.bZ());
        dh2.a("Riches", this.bx);
        if (this.bu != null) {
            dh2.a("Offers", this.bu.a());
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.s(dh2.f("Profession"));
        this.bx = dh2.f("Riches");
        if (dh2.b("Offers", 10)) {
            dh dh3 = dh2.m("Offers");
            this.bu = new ago(dh3);
        }
    }

    @Override
    protected boolean v() {
        return false;
    }

    @Override
    protected String t() {
        if (this.cc()) {
            return "mob.villager.haggle";
        }
        return "mob.villager.idle";
    }

    @Override
    protected String aT() {
        return "mob.villager.hit";
    }

    @Override
    protected String aU() {
        return "mob.villager.death";
    }

    public void s(int n2) {
        this.af.b(16, n2);
    }

    public int bZ() {
        return this.af.c(16);
    }

    public boolean ca() {
        return this.br;
    }

    public void i(boolean bl2) {
        this.br = bl2;
    }

    public void j(boolean bl2) {
        this.bs = bl2;
    }

    public boolean cb() {
        return this.bs;
    }

    @Override
    public void b(sv sv2) {
        super.b(sv2);
        if (this.bp != null && sv2 != null) {
            this.bp.a(sv2);
            if (sv2 instanceof yz) {
                int n2 = -1;
                if (this.f()) {
                    n2 = -3;
                }
                this.bp.a(sv2.b_(), n2);
                if (this.Z()) {
                    this.o.a((sa)this, (byte)13);
                }
            }
        }
    }

    @Override
    public void a(ro ro2) {
        if (this.bp != null) {
            yz yz2;
            sa sa2 = ro2.j();
            if (sa2 != null) {
                if (sa2 instanceof yz) {
                    this.bp.a(sa2.b_(), -2);
                } else if (sa2 instanceof yb) {
                    this.bp.h();
                }
            } else if (sa2 == null && (yz2 = this.o.a((sa)this, 16.0)) != null) {
                this.bp.h();
            }
        }
        super.a(ro2);
    }

    @Override
    public void a_(yz yz2) {
        this.bt = yz2;
    }

    @Override
    public yz b() {
        return this.bt;
    }

    public boolean cc() {
        return this.bt != null;
    }

    @Override
    public void a(agn agn2) {
        agn2.f();
        this.a_ = -this.q();
        this.a("mob.villager.yes", this.bf(), this.bg());
        if (agn2.a((agn)this.bu.get(this.bu.size() - 1))) {
            this.bv = 40;
            this.bw = true;
            this.by = this.bt != null ? this.bt.b_() : null;
        }
        if (agn2.a().b() == ade.bC) {
            this.bx += agn2.a().b;
        }
    }

    @Override
    public void a_(add add2) {
        if (!this.o.E && this.a_ > -this.q() + 20) {
            this.a_ = -this.q();
            if (add2 != null) {
                this.a("mob.villager.yes", this.bf(), this.bg());
            } else {
                this.a("mob.villager.no", this.bf(), this.bg());
            }
        }
    }

    @Override
    public ago b(yz yz2) {
        if (this.bu == null) {
            this.t(1);
        }
        return this.bu;
    }

    private float p(float f2) {
        float f3 = f2 + this.bA;
        if (f3 > 0.9f) {
            return 0.9f - (f3 - 0.9f);
        }
        return f3;
    }

    private void t(int n2) {
        this.bA = this.bu != null ? qh.c(this.bu.size()) * 0.2f : 0.0f;
        ago ago2 = new ago();
        switch (this.bZ()) {
            case 0: {
                yv.a(ago2, ade.O, this.Z, this.p(0.9f));
                yv.a(ago2, adb.a(ajn.L), this.Z, this.p(0.5f));
                yv.a(ago2, ade.bf, this.Z, this.p(0.5f));
                yv.a(ago2, ade.aQ, this.Z, this.p(0.4f));
                yv.b(ago2, ade.P, this.Z, this.p(0.9f));
                yv.b(ago2, ade.ba, this.Z, this.p(0.3f));
                yv.b(ago2, ade.e, this.Z, this.p(0.3f));
                yv.b(ago2, ade.aX, this.Z, this.p(0.3f));
                yv.b(ago2, ade.aZ, this.Z, this.p(0.3f));
                yv.b(ago2, ade.d, this.Z, this.p(0.3f));
                yv.b(ago2, ade.bg, this.Z, this.p(0.3f));
                yv.b(ago2, ade.g, this.Z, this.p(0.5f));
                if (!(this.Z.nextFloat() < this.p(0.5f))) break;
                ago2.add(new agn(new add(ajn.n, 10), new add(ade.bC), new add(ade.ak, 4 + this.Z.nextInt(2), 0)));
                break;
            }
            case 4: {
                yv.a(ago2, ade.h, this.Z, this.p(0.7f));
                yv.a(ago2, ade.al, this.Z, this.p(0.5f));
                yv.a(ago2, ade.bd, this.Z, this.p(0.5f));
                yv.b(ago2, ade.av, this.Z, this.p(0.1f));
                yv.b(ago2, ade.R, this.Z, this.p(0.3f));
                yv.b(ago2, ade.T, this.Z, this.p(0.3f));
                yv.b(ago2, ade.Q, this.Z, this.p(0.3f));
                yv.b(ago2, ade.S, this.Z, this.p(0.3f));
                yv.b(ago2, ade.am, this.Z, this.p(0.3f));
                yv.b(ago2, ade.be, this.Z, this.p(0.3f));
                break;
            }
            case 3: {
                yv.a(ago2, ade.h, this.Z, this.p(0.7f));
                yv.a(ago2, ade.j, this.Z, this.p(0.5f));
                yv.a(ago2, ade.k, this.Z, this.p(0.5f));
                yv.a(ago2, ade.i, this.Z, this.p(0.5f));
                yv.b(ago2, ade.l, this.Z, this.p(0.5f));
                yv.b(ago2, ade.u, this.Z, this.p(0.5f));
                yv.b(ago2, ade.c, this.Z, this.p(0.3f));
                yv.b(ago2, ade.x, this.Z, this.p(0.3f));
                yv.b(ago2, ade.b, this.Z, this.p(0.5f));
                yv.b(ago2, ade.w, this.Z, this.p(0.5f));
                yv.b(ago2, ade.a, this.Z, this.p(0.2f));
                yv.b(ago2, ade.v, this.Z, this.p(0.2f));
                yv.b(ago2, ade.K, this.Z, this.p(0.2f));
                yv.b(ago2, ade.L, this.Z, this.p(0.2f));
                yv.b(ago2, ade.ab, this.Z, this.p(0.2f));
                yv.b(ago2, ade.af, this.Z, this.p(0.2f));
                yv.b(ago2, ade.Y, this.Z, this.p(0.2f));
                yv.b(ago2, ade.ac, this.Z, this.p(0.2f));
                yv.b(ago2, ade.Z, this.Z, this.p(0.2f));
                yv.b(ago2, ade.ad, this.Z, this.p(0.2f));
                yv.b(ago2, ade.aa, this.Z, this.p(0.2f));
                yv.b(ago2, ade.ae, this.Z, this.p(0.2f));
                yv.b(ago2, ade.X, this.Z, this.p(0.1f));
                yv.b(ago2, ade.U, this.Z, this.p(0.1f));
                yv.b(ago2, ade.V, this.Z, this.p(0.1f));
                yv.b(ago2, ade.W, this.Z, this.p(0.1f));
                break;
            }
            case 1: {
                yv.a(ago2, ade.aF, this.Z, this.p(0.8f));
                yv.a(ago2, ade.aG, this.Z, this.p(0.8f));
                yv.a(ago2, ade.bB, this.Z, this.p(0.3f));
                yv.b(ago2, adb.a(ajn.X), this.Z, this.p(0.8f));
                yv.b(ago2, adb.a(ajn.w), this.Z, this.p(0.2f));
                yv.b(ago2, ade.aL, this.Z, this.p(0.2f));
                yv.b(ago2, ade.aN, this.Z, this.p(0.2f));
                if (!(this.Z.nextFloat() < this.p(0.07f))) break;
                Object object = aft.c[this.Z.nextInt(aft.c.length)];
                int n3 = qh.a(this.Z, ((aft)object).d(), ((aft)object).b());
                add add2 = ade.bR.a(new agc((aft)object, n3));
                int n4 = 2 + this.Z.nextInt(5 + n3 * 10) + 3 * n3;
                ago2.add(new agn(new add(ade.aG), new add(ade.bC, n4), add2));
                break;
            }
            case 2: {
                yv.b(ago2, ade.bv, this.Z, this.p(0.3f));
                yv.b(ago2, ade.by, this.Z, this.p(0.2f));
                yv.b(ago2, ade.ax, this.Z, this.p(0.4f));
                yv.b(ago2, adb.a(ajn.aN), this.Z, this.p(0.3f));
                Object object = new adb[]{ade.l, ade.u, ade.Z, ade.ad, ade.c, ade.x, ade.b, ade.w};
                for (adb adb2 : object) {
                    if (!(this.Z.nextFloat() < this.p(0.05f))) continue;
                    ago2.add(new agn(new add(adb2, 1, 0), new add(ade.bC, 2 + this.Z.nextInt(3), 0), afv.a(this.Z, new add(adb2, 1, 0), 5 + this.Z.nextInt(15))));
                }
                break;
            }
        }
        if (ago2.isEmpty()) {
            yv.a(ago2, ade.k, this.Z, 1.0f);
        }
        Collections.shuffle(ago2);
        if (this.bu == null) {
            this.bu = new ago();
        }
        for (int i2 = 0; i2 < n2 && i2 < ago2.size(); ++i2) {
            this.bu.a((agn)ago2.get(i2));
        }
    }

    private static void a(ago ago2, adb adb2, Random random, float f2) {
        if (random.nextFloat() < f2) {
            ago2.add(new agn(yv.a(adb2, random), ade.bC));
        }
    }

    private static add a(adb adb2, Random random) {
        return new add(adb2, yv.b(adb2, random), 0);
    }

    private static int b(adb adb2, Random random) {
        qu qu2 = (qu)bB.get(adb2);
        if (qu2 == null) {
            return 1;
        }
        if ((Integer)qu2.a() >= (Integer)qu2.b()) {
            return (Integer)qu2.a();
        }
        return (Integer)qu2.a() + random.nextInt((Integer)qu2.b() - (Integer)qu2.a());
    }

    private static void b(ago ago2, adb adb2, Random random, float f2) {
        if (random.nextFloat() < f2) {
            add add2;
            add add3;
            int n2 = yv.c(adb2, random);
            if (n2 < 0) {
                add3 = new add(ade.bC, 1, 0);
                add2 = new add(adb2, -n2, 0);
            } else {
                add3 = new add(ade.bC, n2, 0);
                add2 = new add(adb2, 1, 0);
            }
            ago2.add(new agn(add3, add2));
        }
    }

    private static int c(adb adb2, Random random) {
        qu qu2 = (qu)bC.get(adb2);
        if (qu2 == null) {
            return 1;
        }
        if ((Integer)qu2.a() >= (Integer)qu2.b()) {
            return (Integer)qu2.a();
        }
        return (Integer)qu2.a() + random.nextInt((Integer)qu2.b() - (Integer)qu2.a());
    }

    @Override
    public sy a(sy sy2) {
        sy2 = super.a(sy2);
        this.s(this.o.s.nextInt(5));
        return sy2;
    }

    public void cd() {
        this.bz = true;
    }

    public yv b(rx rx2) {
        yv yv2 = new yv(this.o);
        yv2.a((sy)null);
        return yv2;
    }

    @Override
    public boolean bM() {
        return false;
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.b(rx2);
    }

    static {
        bB.put(ade.h, new qu(16, 24));
        bB.put(ade.j, new qu(8, 10));
        bB.put(ade.k, new qu(8, 10));
        bB.put(ade.i, new qu(4, 6));
        bB.put(ade.aF, new qu(24, 36));
        bB.put(ade.aG, new qu(11, 13));
        bB.put(ade.bB, new qu(1, 1));
        bB.put(ade.bi, new qu(3, 4));
        bB.put(ade.bv, new qu(2, 3));
        bB.put(ade.al, new qu(14, 18));
        bB.put(ade.bd, new qu(14, 18));
        bB.put(ade.bf, new qu(14, 18));
        bB.put(ade.aQ, new qu(9, 13));
        bB.put(ade.N, new qu(34, 48));
        bB.put(ade.bc, new qu(30, 38));
        bB.put(ade.bb, new qu(30, 38));
        bB.put(ade.O, new qu(18, 22));
        bB.put(adb.a(ajn.L), new qu(14, 22));
        bB.put(ade.bh, new qu(36, 64));
        bC.put(ade.d, new qu(3, 4));
        bC.put(ade.aZ, new qu(3, 4));
        bC.put(ade.l, new qu(7, 11));
        bC.put(ade.u, new qu(12, 14));
        bC.put(ade.c, new qu(6, 8));
        bC.put(ade.x, new qu(9, 12));
        bC.put(ade.b, new qu(7, 9));
        bC.put(ade.w, new qu(10, 12));
        bC.put(ade.a, new qu(4, 6));
        bC.put(ade.v, new qu(7, 8));
        bC.put(ade.K, new qu(4, 6));
        bC.put(ade.L, new qu(7, 8));
        bC.put(ade.ab, new qu(4, 6));
        bC.put(ade.af, new qu(7, 8));
        bC.put(ade.Y, new qu(4, 6));
        bC.put(ade.ac, new qu(7, 8));
        bC.put(ade.Z, new qu(10, 14));
        bC.put(ade.ad, new qu(16, 19));
        bC.put(ade.aa, new qu(8, 10));
        bC.put(ade.ae, new qu(11, 14));
        bC.put(ade.X, new qu(5, 7));
        bC.put(ade.U, new qu(5, 7));
        bC.put(ade.V, new qu(11, 15));
        bC.put(ade.W, new qu(9, 11));
        bC.put(ade.P, new qu(-4, -2));
        bC.put(ade.ba, new qu(-8, -4));
        bC.put(ade.e, new qu(-8, -4));
        bC.put(ade.aX, new qu(-10, -7));
        bC.put(adb.a(ajn.w), new qu(-5, -3));
        bC.put(adb.a(ajn.X), new qu(3, 4));
        bC.put(ade.R, new qu(4, 5));
        bC.put(ade.T, new qu(2, 4));
        bC.put(ade.Q, new qu(2, 4));
        bC.put(ade.S, new qu(2, 4));
        bC.put(ade.av, new qu(6, 8));
        bC.put(ade.by, new qu(-4, -1));
        bC.put(ade.ax, new qu(-4, -1));
        bC.put(ade.aL, new qu(10, 12));
        bC.put(ade.aN, new qu(10, 12));
        bC.put(adb.a(ajn.aN), new qu(-3, -1));
        bC.put(ade.am, new qu(-7, -5));
        bC.put(ade.be, new qu(-7, -5));
        bC.put(ade.bg, new qu(-8, -6));
        bC.put(ade.bv, new qu(7, 11));
        bC.put(ade.g, new qu(-12, -8));
    }
}

