/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.UUID;

public abstract class sw
extends sv {
    public int a_;
    protected int b;
    private tu h;
    private tv i;
    private tt bm;
    private tr bn;
    private vv bo;
    protected final uj c;
    protected final uj d;
    private sv bp;
    private vw bq;
    private add[] br = new add[5];
    protected float[] e = new float[5];
    private boolean bs;
    private boolean bt;
    protected float f;
    private sa bu;
    protected int g;
    private boolean bv;
    private sa bw;
    private dh bx;

    public sw(ahb ahb2) {
        super(ahb2);
        this.c = new uj(ahb2 == null || ahb2.C == null ? null : ahb2.C);
        this.d = new uj(ahb2 == null || ahb2.C == null ? null : ahb2.C);
        this.h = new tu(this);
        this.i = new tv(this);
        this.bm = new tt(this);
        this.bn = new tr(this);
        this.bo = new vv(this, ahb2);
        this.bq = new vw(this);
        for (int i2 = 0; i2 < this.e.length; ++i2) {
            this.e[i2] = 0.085f;
        }
    }

    @Override
    protected void aD() {
        super.aD();
        this.bc().b(yj.b).a(16.0);
    }

    public tu j() {
        return this.h;
    }

    public tv k() {
        return this.i;
    }

    public tt l() {
        return this.bm;
    }

    public vv m() {
        return this.bo;
    }

    public vw n() {
        return this.bq;
    }

    public sv o() {
        return this.bp;
    }

    public void d(sv sv2) {
        this.bp = sv2;
    }

    public boolean a(Class clazz) {
        return xz.class != clazz && yd.class != clazz;
    }

    public void p() {
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(11, (Object)0);
        this.af.a(10, "");
    }

    public int q() {
        return 80;
    }

    public void r() {
        String string = this.t();
        if (string != null) {
            this.a(string, this.bf(), this.bg());
        }
    }

    @Override
    public void C() {
        super.C();
        this.o.C.a("mobBaseTick");
        if (this.Z() && this.Z.nextInt(1000) < this.a_++) {
            this.a_ = -this.q();
            this.r();
        }
        this.o.C.b();
    }

    @Override
    protected int e(yz yz2) {
        if (this.b > 0) {
            int n2 = this.b;
            add[] addArray = this.ak();
            for (int i2 = 0; i2 < addArray.length; ++i2) {
                if (addArray[i2] == null || !(this.e[i2] <= 1.0f)) continue;
                n2 += 1 + this.Z.nextInt(3);
            }
            return n2;
        }
        return this.b;
    }

    public void s() {
        for (int i2 = 0; i2 < 20; ++i2) {
            double d2 = this.Z.nextGaussian() * 0.02;
            double d3 = this.Z.nextGaussian() * 0.02;
            double d4 = this.Z.nextGaussian() * 0.02;
            double d5 = 10.0;
            this.o.a("explode", this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M - d2 * d5, this.t + (double)(this.Z.nextFloat() * this.N) - d3 * d5, this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M - d4 * d5, d2, d3, d4);
        }
    }

    @Override
    public void h() {
        super.h();
        if (!this.o.E) {
            this.bL();
        }
    }

    @Override
    protected float f(float f2, float f3) {
        if (this.bk()) {
            this.bn.a();
            return f3;
        }
        return super.f(f2, f3);
    }

    protected String t() {
        return null;
    }

    protected adb u() {
        return adb.d(0);
    }

    @Override
    protected void b(boolean bl2, int n2) {
        adb adb2 = this.u();
        if (adb2 != null) {
            int n3 = this.Z.nextInt(3);
            if (n2 > 0) {
                n3 += this.Z.nextInt(n2 + 1);
            }
            for (int i2 = 0; i2 < n3; ++i2) {
                this.a(adb2, 1);
            }
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("CanPickUpLoot", this.bJ());
        dh2.a("PersistenceRequired", this.bt);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.br.length; ++i2) {
            dh dh3 = new dh();
            if (this.br[i2] != null) {
                this.br[i2].b(dh3);
            }
            dq2.a(dh3);
        }
        dh2.a("Equipment", dq2);
        dq dq3 = new dq();
        for (int i3 = 0; i3 < this.e.length; ++i3) {
            dq3.a(new dm(this.e[i3]));
        }
        dh2.a("DropChances", dq3);
        dh2.a("CustomName", this.bG());
        dh2.a("CustomNameVisible", this.bI());
        dh2.a("Leashed", this.bv);
        if (this.bw != null) {
            dh dh4 = new dh();
            if (this.bw instanceof sv) {
                dh4.a("UUIDMost", this.bw.aB().getMostSignificantBits());
                dh4.a("UUIDLeast", this.bw.aB().getLeastSignificantBits());
            } else if (this.bw instanceof ss) {
                ss ss2 = (ss)this.bw;
                dh4.a("X", ss2.b);
                dh4.a("Y", ss2.c);
                dh4.a("Z", ss2.d);
            }
            dh2.a("Leash", dh4);
        }
    }

    @Override
    public void a(dh dh2) {
        int n2;
        dq dq2;
        super.a(dh2);
        this.h(dh2.n("CanPickUpLoot"));
        this.bt = dh2.n("PersistenceRequired");
        if (dh2.b("CustomName", 8) && dh2.j("CustomName").length() > 0) {
            this.a(dh2.j("CustomName"));
        }
        this.g(dh2.n("CustomNameVisible"));
        if (dh2.b("Equipment", 9)) {
            dq2 = dh2.c("Equipment", 10);
            for (n2 = 0; n2 < this.br.length; ++n2) {
                this.br[n2] = add.a(dq2.b(n2));
            }
        }
        if (dh2.b("DropChances", 9)) {
            dq2 = dh2.c("DropChances", 5);
            for (n2 = 0; n2 < dq2.c(); ++n2) {
                this.e[n2] = dq2.e(n2);
            }
        }
        this.bv = dh2.n("Leashed");
        if (this.bv && dh2.b("Leash", 10)) {
            this.bx = dh2.m("Leash");
        }
    }

    public void n(float f2) {
        this.be = f2;
    }

    @Override
    public void i(float f2) {
        super.i(f2);
        this.n(f2);
    }

    @Override
    public void e() {
        super.e();
        this.o.C.a("looting");
        if (!this.o.E && this.bJ() && !this.aT && this.o.O().b("mobGriefing")) {
            List list = this.o.a(xk.class, this.C.b(1.0, 0.0, 1.0));
            for (xk xk2 : list) {
                Object object;
                add add2;
                int n2;
                if (xk2.K || xk2.f() == null || (n2 = sw.b(add2 = xk2.f())) <= -1) continue;
                boolean bl2 = true;
                add add3 = this.q(n2);
                if (add3 != null) {
                    adb adb2;
                    if (n2 == 0) {
                        if (add2.b() instanceof aeh && !(add3.b() instanceof aeh)) {
                            bl2 = true;
                        } else if (add2.b() instanceof aeh && add3.b() instanceof aeh) {
                            object = (aeh)add2.b();
                            adb2 = (aeh)add3.b();
                            bl2 = ((aeh)object).i() == ((aeh)adb2).i() ? add2.k() > add3.k() || add2.p() && !add3.p() : ((aeh)object).i() > ((aeh)adb2).i();
                        } else {
                            bl2 = false;
                        }
                    } else if (add2.b() instanceof abb && !(add3.b() instanceof abb)) {
                        bl2 = true;
                    } else if (add2.b() instanceof abb && add3.b() instanceof abb) {
                        object = (abb)add2.b();
                        adb2 = (abb)add3.b();
                        bl2 = ((abb)object).c == ((abb)adb2).c ? add2.k() > add3.k() || add2.p() && !add3.p() : ((abb)object).c > ((abb)adb2).c;
                    } else {
                        bl2 = false;
                    }
                }
                if (!bl2) continue;
                if (add3 != null && this.Z.nextFloat() - 0.1f < this.e[n2]) {
                    this.a(add3, 0.0f);
                }
                if (add2.b() == ade.i && xk2.j() != null && (object = this.o.a(xk2.j())) != null) {
                    ((yz)object).a(pc.x);
                }
                this.c(n2, add2);
                this.e[n2] = 2.0f;
                this.bt = true;
                this.a((sa)xk2, 1);
                xk2.B();
            }
        }
        this.o.C.b();
    }

    @Override
    protected boolean bk() {
        return false;
    }

    protected boolean v() {
        return true;
    }

    protected void w() {
        if (this.bt) {
            this.aU = 0;
            return;
        }
        yz yz2 = this.o.a((sa)this, -1.0);
        if (yz2 != null) {
            double d2 = yz2.s - this.s;
            double d3 = yz2.t - this.t;
            double d4 = yz2.u - this.u;
            double d5 = d2 * d2 + d3 * d3 + d4 * d4;
            if (this.v() && d5 > 16384.0) {
                this.B();
            }
            if (this.aU > 600 && this.Z.nextInt(800) == 0 && d5 > 1024.0 && this.v()) {
                this.B();
            } else if (d5 < 1024.0) {
                this.aU = 0;
            }
        }
    }

    @Override
    protected void bn() {
        ++this.aU;
        this.o.C.a("checkDespawn");
        this.w();
        this.o.C.b();
        this.o.C.a("sensing");
        this.bq.a();
        this.o.C.b();
        this.o.C.a("targetSelector");
        this.d.a();
        this.o.C.b();
        this.o.C.a("goalSelector");
        this.c.a();
        this.o.C.b();
        this.o.C.a("navigation");
        this.bo.f();
        this.o.C.b();
        this.o.C.a("mob tick");
        this.bp();
        this.o.C.b();
        this.o.C.a("controls");
        this.o.C.a("move");
        this.i.c();
        this.o.C.c("look");
        this.h.a();
        this.o.C.c("jump");
        this.bm.b();
        this.o.C.b();
        this.o.C.b();
    }

    @Override
    protected void bq() {
        super.bq();
        this.bd = 0.0f;
        this.be = 0.0f;
        this.w();
        float f2 = 8.0f;
        if (this.Z.nextFloat() < 0.02f) {
            yz yz2 = this.o.a((sa)this, f2);
            if (yz2 != null) {
                this.bu = yz2;
                this.g = 10 + this.Z.nextInt(20);
            } else {
                this.bf = (this.Z.nextFloat() - 0.5f) * 20.0f;
            }
        }
        if (this.bu != null) {
            this.a(this.bu, 10.0f, (float)this.x());
            if (this.g-- <= 0 || this.bu.K || this.bu.f(this) > (double)(f2 * f2)) {
                this.bu = null;
            }
        } else {
            if (this.Z.nextFloat() < 0.05f) {
                this.bf = (this.Z.nextFloat() - 0.5f) * 20.0f;
            }
            this.y += this.bf;
            this.z = this.f;
        }
        boolean bl2 = this.M();
        boolean bl3 = this.P();
        if (bl2 || bl3) {
            this.bc = this.Z.nextFloat() < 0.8f;
        }
    }

    public int x() {
        return 40;
    }

    public void a(sa sa2, float f2, float f3) {
        double d2;
        double d3 = sa2.s - this.s;
        double d4 = sa2.u - this.u;
        if (sa2 instanceof sv) {
            sv sv2 = (sv)sa2;
            d2 = sv2.t + (double)sv2.g() - (this.t + (double)this.g());
        } else {
            d2 = (sa2.C.b + sa2.C.e) / 2.0 - (this.t + (double)this.g());
        }
        double d5 = qh.a(d3 * d3 + d4 * d4);
        float f4 = (float)(Math.atan2(d4, d3) * 180.0 / 3.1415927410125732) - 90.0f;
        float f5 = (float)(-(Math.atan2(d2, d5) * 180.0 / 3.1415927410125732));
        this.z = this.b(this.z, f5, f3);
        this.y = this.b(this.y, f4, f2);
    }

    private float b(float f2, float f3, float f4) {
        float f5 = qh.g(f3 - f2);
        if (f5 > f4) {
            f5 = f4;
        }
        if (f5 < -f4) {
            f5 = -f4;
        }
        return f2 + f5;
    }

    public boolean by() {
        return this.o.b(this.C) && this.o.a((sa)this, this.C).isEmpty() && !this.o.d(this.C);
    }

    public int bB() {
        return 4;
    }

    @Override
    public int ax() {
        if (this.o() == null) {
            return 3;
        }
        int n2 = (int)(this.aS() - this.aY() * 0.33f);
        if ((n2 -= (3 - this.o.r.a()) * 4) < 0) {
            n2 = 0;
        }
        return n2 + 3;
    }

    @Override
    public add be() {
        return this.br[0];
    }

    @Override
    public add q(int n2) {
        return this.br[n2];
    }

    public add r(int n2) {
        return this.br[n2 + 1];
    }

    @Override
    public void c(int n2, add add2) {
        this.br[n2] = add2;
    }

    @Override
    public add[] ak() {
        return this.br;
    }

    @Override
    protected void a(boolean bl2, int n2) {
        for (int i2 = 0; i2 < this.ak().length; ++i2) {
            boolean bl3;
            add add2 = this.q(i2);
            boolean bl4 = bl3 = this.e[i2] > 1.0f;
            if (add2 == null || !bl2 && !bl3 || !(this.Z.nextFloat() - (float)n2 * 0.01f < this.e[i2])) continue;
            if (!bl3 && add2.g()) {
                int n3 = Math.max(add2.l() - 25, 1);
                int n4 = add2.l() - this.Z.nextInt(this.Z.nextInt(n3) + 1);
                if (n4 > n3) {
                    n4 = n3;
                }
                if (n4 < 1) {
                    n4 = 1;
                }
                add2.b(n4);
            }
            this.a(add2, 0.0f);
        }
    }

    protected void bC() {
        if (this.Z.nextFloat() < 0.15f * this.o.b(this.s, this.t, this.u)) {
            float f2;
            int n2 = this.Z.nextInt(2);
            float f3 = f2 = this.o.r == rd.d ? 0.1f : 0.25f;
            if (this.Z.nextFloat() < 0.095f) {
                ++n2;
            }
            if (this.Z.nextFloat() < 0.095f) {
                ++n2;
            }
            if (this.Z.nextFloat() < 0.095f) {
                ++n2;
            }
            for (int i2 = 3; i2 >= 0; --i2) {
                adb adb2;
                add add2 = this.r(i2);
                if (i2 < 3 && this.Z.nextFloat() < f2) break;
                if (add2 != null || (adb2 = sw.a(i2 + 1, n2)) == null) continue;
                this.c(i2 + 1, new add(adb2));
            }
        }
    }

    public static int b(add add2) {
        if (add2.b() == adb.a(ajn.aK) || add2.b() == ade.bL) {
            return 4;
        }
        if (add2.b() instanceof abb) {
            switch (((abb)add2.b()).b) {
                case 3: {
                    return 1;
                }
                case 2: {
                    return 2;
                }
                case 1: {
                    return 3;
                }
                case 0: {
                    return 4;
                }
            }
        }
        return 0;
    }

    public static adb a(int n2, int n3) {
        switch (n2) {
            case 4: {
                if (n3 == 0) {
                    return ade.Q;
                }
                if (n3 == 1) {
                    return ade.ag;
                }
                if (n3 == 2) {
                    return ade.U;
                }
                if (n3 == 3) {
                    return ade.Y;
                }
                if (n3 == 4) {
                    return ade.ac;
                }
            }
            case 3: {
                if (n3 == 0) {
                    return ade.R;
                }
                if (n3 == 1) {
                    return ade.ah;
                }
                if (n3 == 2) {
                    return ade.V;
                }
                if (n3 == 3) {
                    return ade.Z;
                }
                if (n3 == 4) {
                    return ade.ad;
                }
            }
            case 2: {
                if (n3 == 0) {
                    return ade.S;
                }
                if (n3 == 1) {
                    return ade.ai;
                }
                if (n3 == 2) {
                    return ade.W;
                }
                if (n3 == 3) {
                    return ade.aa;
                }
                if (n3 == 4) {
                    return ade.ae;
                }
            }
            case 1: {
                if (n3 == 0) {
                    return ade.T;
                }
                if (n3 == 1) {
                    return ade.aj;
                }
                if (n3 == 2) {
                    return ade.X;
                }
                if (n3 == 3) {
                    return ade.ab;
                }
                if (n3 != 4) break;
                return ade.af;
            }
        }
        return null;
    }

    protected void bD() {
        float f2 = this.o.b(this.s, this.t, this.u);
        if (this.be() != null && this.Z.nextFloat() < 0.25f * f2) {
            afv.a(this.Z, this.be(), (int)(5.0f + f2 * (float)this.Z.nextInt(18)));
        }
        for (int i2 = 0; i2 < 4; ++i2) {
            add add2 = this.r(i2);
            if (add2 == null || !(this.Z.nextFloat() < 0.5f * f2)) continue;
            afv.a(this.Z, add2, (int)(5.0f + f2 * (float)this.Z.nextInt(18)));
        }
    }

    public sy a(sy sy2) {
        this.a(yj.b).a(new tj("Random spawn bonus", this.Z.nextGaussian() * 0.05, 1));
        return sy2;
    }

    public boolean bE() {
        return false;
    }

    @Override
    public String b_() {
        if (this.bH()) {
            return this.bG();
        }
        return super.b_();
    }

    public void bF() {
        this.bt = true;
    }

    public void a(String string) {
        this.af.b(10, string);
    }

    public String bG() {
        return this.af.e(10);
    }

    public boolean bH() {
        return this.af.e(10).length() > 0;
    }

    public void g(boolean bl2) {
        this.af.b(11, bl2 ? (byte)1 : 0);
    }

    public boolean bI() {
        return this.af.a(11) == 1;
    }

    public void a(int n2, float f2) {
        this.e[n2] = f2;
    }

    public boolean bJ() {
        return this.bs;
    }

    public void h(boolean bl2) {
        this.bs = bl2;
    }

    public boolean bK() {
        return this.bt;
    }

    @Override
    public final boolean c(yz yz2) {
        if (this.bN() && this.bO() == yz2) {
            this.a(true, !yz2.bE.d);
            return true;
        }
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.ca && this.bM()) {
            if (this instanceof tg && ((tg)this).bZ()) {
                if (((tg)this).e((sv)yz2)) {
                    this.b((sa)yz2, true);
                    --add2.b;
                    return true;
                }
            } else {
                this.b((sa)yz2, true);
                --add2.b;
                return true;
            }
        }
        if (this.a(yz2)) {
            return true;
        }
        return super.c(yz2);
    }

    protected boolean a(yz yz2) {
        return false;
    }

    protected void bL() {
        if (this.bx != null) {
            this.bP();
        }
        if (!this.bv) {
            return;
        }
        if (this.bw == null || this.bw.K) {
            this.a(true, true);
            return;
        }
    }

    public void a(boolean bl2, boolean bl3) {
        if (this.bv) {
            this.bv = false;
            this.bw = null;
            if (!this.o.E && bl3) {
                this.a(ade.ca, 1);
            }
            if (!this.o.E && bl2 && this.o instanceof mt) {
                ((mt)this.o).r().a(this, new hx(1, this, null));
            }
        }
    }

    public boolean bM() {
        return !this.bN() && !(this instanceof yb);
    }

    public boolean bN() {
        return this.bv;
    }

    public sa bO() {
        return this.bw;
    }

    public void b(sa sa2, boolean bl2) {
        this.bv = true;
        this.bw = sa2;
        if (!this.o.E && bl2 && this.o instanceof mt) {
            ((mt)this.o).r().a(this, new hx(1, this, this.bw));
        }
    }

    private void bP() {
        if (this.bv && this.bx != null) {
            if (this.bx.b("UUIDMost", 4) && this.bx.b("UUIDLeast", 4)) {
                UUID uUID = new UUID(this.bx.g("UUIDMost"), this.bx.g("UUIDLeast"));
                List list = this.o.a(sv.class, this.C.b(10.0, 10.0, 10.0));
                for (sv sv2 : list) {
                    if (!sv2.aB().equals(uUID)) continue;
                    this.bw = sv2;
                    break;
                }
            } else if (this.bx.b("X", 99) && this.bx.b("Y", 99) && this.bx.b("Z", 99)) {
                int n2;
                int n3;
                int n4 = this.bx.f("X");
                su su2 = su.b(this.o, n4, n3 = this.bx.f("Y"), n2 = this.bx.f("Z"));
                if (su2 == null) {
                    su2 = su.a(this.o, n4, n3, n2);
                }
                this.bw = su2;
            } else {
                this.a(false, true);
            }
        }
        this.bx = null;
    }
}

