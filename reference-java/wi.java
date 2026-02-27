/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class wi
extends wf
implements rc {
    private static final sj bu = new wj();
    private static final th bv = new to("horse.jumpStrength", 0.7, 0.0, 2.0).a("Jump Strength").a(true);
    private static final String[] bw = new String[]{null, "textures/entity/horse/armor/horse_armor_iron.png", "textures/entity/horse/armor/horse_armor_gold.png", "textures/entity/horse/armor/horse_armor_diamond.png"};
    private static final String[] bx = new String[]{"", "meo", "goo", "dio"};
    private static final int[] by = new int[]{0, 5, 7, 11};
    private static final String[] bz = new String[]{"textures/entity/horse/horse_white.png", "textures/entity/horse/horse_creamy.png", "textures/entity/horse/horse_chestnut.png", "textures/entity/horse/horse_brown.png", "textures/entity/horse/horse_black.png", "textures/entity/horse/horse_gray.png", "textures/entity/horse/horse_darkbrown.png"};
    private static final String[] bA = new String[]{"hwh", "hcr", "hch", "hbr", "hbl", "hgr", "hdb"};
    private static final String[] bB = new String[]{null, "textures/entity/horse/horse_markings_white.png", "textures/entity/horse/horse_markings_whitefield.png", "textures/entity/horse/horse_markings_whitedots.png", "textures/entity/horse/horse_markings_blackdots.png"};
    private static final String[] bC = new String[]{"", "wo_", "wmo", "wdo", "bdo"};
    private int bD;
    private int bE;
    private int bF;
    public int bp;
    public int bq;
    protected boolean br;
    private zt bG;
    private boolean bH;
    protected int bs;
    protected float bt;
    private boolean bI;
    private float bJ;
    private float bK;
    private float bL;
    private float bM;
    private float bN;
    private float bO;
    private int bP;
    private String bQ;
    private String[] bR = new String[3];

    public wi(ahb ahb2) {
        super(ahb2);
        this.a(1.4f, 1.6f);
        this.ae = false;
        this.l(false);
        this.m().a(true);
        this.c.a(0, new uf(this));
        this.c.a(1, new uz(this, 1.2));
        this.c.a(1, new vg(this, 1.2));
        this.c.a(2, new ua(this, 1.0));
        this.c.a(4, new uh(this, 1.0));
        this.c.a(6, new vc(this, 0.7));
        this.c.a(7, new un(this, yz.class, 6.0f));
        this.c.a(8, new vb(this));
        this.cN();
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
        this.af.a(19, (Object)0);
        this.af.a(20, (Object)0);
        this.af.a(21, String.valueOf(""));
        this.af.a(22, (Object)0);
    }

    public void s(int n2) {
        this.af.b(19, (byte)n2);
        this.cP();
    }

    public int bZ() {
        return this.af.a(19);
    }

    public void t(int n2) {
        this.af.b(20, n2);
        this.cP();
    }

    public int ca() {
        return this.af.c(20);
    }

    @Override
    public String b_() {
        if (this.bH()) {
            return this.bG();
        }
        int n2 = this.bZ();
        switch (n2) {
            default: {
                return dd.a("entity.horse.name");
            }
            case 1: {
                return dd.a("entity.donkey.name");
            }
            case 2: {
                return dd.a("entity.mule.name");
            }
            case 4: {
                return dd.a("entity.skeletonhorse.name");
            }
            case 3: 
        }
        return dd.a("entity.zombiehorse.name");
    }

    private boolean x(int n2) {
        return (this.af.c(16) & n2) != 0;
    }

    private void b(int n2, boolean bl2) {
        int n3 = this.af.c(16);
        if (bl2) {
            this.af.b(16, n3 | n2);
        } else {
            this.af.b(16, n3 & ~n2);
        }
    }

    public boolean cb() {
        return !this.f();
    }

    public boolean cc() {
        return this.x(2);
    }

    public boolean cg() {
        return this.cb();
    }

    public String ch() {
        return this.af.e(21);
    }

    public void b(String string) {
        this.af.b(21, string);
    }

    public float ci() {
        int n2 = this.d();
        if (n2 >= 0) {
            return 1.0f;
        }
        return 0.5f + (float)(-24000 - n2) / -24000.0f * 0.5f;
    }

    @Override
    public void a(boolean bl2) {
        if (bl2) {
            this.a(this.ci());
        } else {
            this.a(1.0f);
        }
    }

    public boolean cj() {
        return this.br;
    }

    public void i(boolean bl2) {
        this.b(2, bl2);
    }

    public void j(boolean bl2) {
        this.br = bl2;
    }

    @Override
    public boolean bM() {
        return !this.cE() && super.bM();
    }

    @Override
    protected void o(float f2) {
        if (f2 > 6.0f && this.cm()) {
            this.o(false);
        }
    }

    public boolean ck() {
        return this.x(8);
    }

    public int cl() {
        return this.af.c(22);
    }

    private int e(add add2) {
        if (add2 == null) {
            return 0;
        }
        adb adb2 = add2.b();
        if (adb2 == ade.bX) {
            return 1;
        }
        if (adb2 == ade.bY) {
            return 2;
        }
        if (adb2 == ade.bZ) {
            return 3;
        }
        return 0;
    }

    public boolean cm() {
        return this.x(32);
    }

    public boolean cn() {
        return this.x(64);
    }

    public boolean co() {
        return this.x(16);
    }

    public boolean cp() {
        return this.bH;
    }

    public void d(add add2) {
        this.af.b(22, this.e(add2));
        this.cP();
    }

    public void k(boolean bl2) {
        this.b(16, bl2);
    }

    public void l(boolean bl2) {
        this.b(8, bl2);
    }

    public void m(boolean bl2) {
        this.bH = bl2;
    }

    public void n(boolean bl2) {
        this.b(4, bl2);
    }

    public int cq() {
        return this.bs;
    }

    public void u(int n2) {
        this.bs = n2;
    }

    public int v(int n2) {
        int n3 = qh.a(this.cq() + n2, 0, this.cw());
        this.u(n3);
        return n3;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        sa sa2 = ro2.j();
        if (this.l != null && this.l.equals(sa2)) {
            return false;
        }
        return super.a(ro2, f2);
    }

    @Override
    public int aV() {
        return by[this.cl()];
    }

    @Override
    public boolean S() {
        return this.l == null;
    }

    public boolean cr() {
        int n2 = qh.c(this.s);
        int n3 = qh.c(this.u);
        this.o.a(n2, n3);
        return true;
    }

    public void cs() {
        if (this.o.E || !this.ck()) {
            return;
        }
        this.a(adb.a(ajn.ae), 1);
        this.l(false);
    }

    private void cL() {
        this.cS();
        this.o.a(this, "eating", 1.0f, 1.0f + (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f);
    }

    @Override
    protected void b(float f2) {
        aji aji2;
        int n2;
        if (f2 > 1.0f) {
            this.a("mob.horse.land", 0.4f, 1.0f);
        }
        if ((n2 = qh.f(f2 * 0.5f - 3.0f)) <= 0) {
            return;
        }
        this.a(ro.h, (float)n2);
        if (this.l != null) {
            this.l.a(ro.h, (float)n2);
        }
        if ((aji2 = this.o.a(qh.c(this.s), qh.c(this.t - 0.2 - (double)this.A), qh.c(this.u))).o() != awt.a) {
            ajm ajm2 = aji2.H;
            this.o.a(this, ajm2.e(), ajm2.c() * 0.5f, ajm2.d() * 0.75f);
        }
    }

    private int cM() {
        int n2 = this.bZ();
        if (this.ck() && (n2 == 1 || n2 == 2)) {
            return 17;
        }
        return 2;
    }

    private void cN() {
        zt zt2 = this.bG;
        this.bG = new zt("HorseChest", this.cM());
        this.bG.a(this.b_());
        if (zt2 != null) {
            zt2.b(this);
            int n2 = Math.min(zt2.a(), this.bG.a());
            for (int i2 = 0; i2 < n2; ++i2) {
                add add2 = zt2.a(i2);
                if (add2 == null) continue;
                this.bG.a(i2, add2.m());
            }
            zt2 = null;
        }
        this.bG.a(this);
        this.cO();
    }

    private void cO() {
        if (!this.o.E) {
            this.n(this.bG.a(0) != null);
            if (this.cB()) {
                this.d(this.bG.a(1));
            }
        }
    }

    @Override
    public void a(rh rh2) {
        int n2 = this.cl();
        boolean bl2 = this.cu();
        this.cO();
        if (this.aa > 20) {
            if (n2 == 0 && n2 != this.cl()) {
                this.a("mob.horse.armor", 0.5f, 1.0f);
            } else if (n2 != this.cl()) {
                this.a("mob.horse.armor", 0.5f, 1.0f);
            }
            if (!bl2 && this.cu()) {
                this.a("mob.horse.leather", 0.5f, 1.0f);
            }
        }
    }

    @Override
    public boolean by() {
        this.cr();
        return super.by();
    }

    protected wi a(sa sa2, double d2) {
        double d3 = Double.MAX_VALUE;
        sa sa3 = null;
        List list = this.o.a(sa2, sa2.C.a(d2, d2, d2), bu);
        for (sa sa4 : list) {
            double d4 = sa4.e(sa2.s, sa2.t, sa2.u);
            if (!(d4 < d3)) continue;
            sa3 = sa4;
            d3 = d4;
        }
        return (wi)sa3;
    }

    public double ct() {
        return this.a(bv).e();
    }

    @Override
    protected String aU() {
        this.cS();
        int n2 = this.bZ();
        if (n2 == 3) {
            return "mob.horse.zombie.death";
        }
        if (n2 == 4) {
            return "mob.horse.skeleton.death";
        }
        if (n2 == 1 || n2 == 2) {
            return "mob.horse.donkey.death";
        }
        return "mob.horse.death";
    }

    @Override
    protected adb u() {
        boolean bl2 = this.Z.nextInt(4) == 0;
        int n2 = this.bZ();
        if (n2 == 4) {
            return ade.aS;
        }
        if (n2 == 3) {
            if (bl2) {
                return adb.d(0);
            }
            return ade.bh;
        }
        return ade.aA;
    }

    @Override
    protected String aT() {
        int n2;
        this.cS();
        if (this.Z.nextInt(3) == 0) {
            this.cU();
        }
        if ((n2 = this.bZ()) == 3) {
            return "mob.horse.zombie.hit";
        }
        if (n2 == 4) {
            return "mob.horse.skeleton.hit";
        }
        if (n2 == 1 || n2 == 2) {
            return "mob.horse.donkey.hit";
        }
        return "mob.horse.hit";
    }

    public boolean cu() {
        return this.x(4);
    }

    @Override
    protected String t() {
        int n2;
        this.cS();
        if (this.Z.nextInt(10) == 0 && !this.bh()) {
            this.cU();
        }
        if ((n2 = this.bZ()) == 3) {
            return "mob.horse.zombie.idle";
        }
        if (n2 == 4) {
            return "mob.horse.skeleton.idle";
        }
        if (n2 == 1 || n2 == 2) {
            return "mob.horse.donkey.idle";
        }
        return "mob.horse.idle";
    }

    protected String cv() {
        this.cS();
        this.cU();
        int n2 = this.bZ();
        if (n2 == 3 || n2 == 4) {
            return null;
        }
        if (n2 == 1 || n2 == 2) {
            return "mob.horse.donkey.angry";
        }
        return "mob.horse.angry";
    }

    @Override
    protected void a(int n2, int n3, int n4, aji aji2) {
        ajm ajm2 = aji2.H;
        if (this.o.a(n2, n3 + 1, n4) == ajn.aC) {
            ajm2 = ajn.aC.H;
        }
        if (!aji2.o().d()) {
            int n5 = this.bZ();
            if (this.l != null && n5 != 1 && n5 != 2) {
                ++this.bP;
                if (this.bP > 5 && this.bP % 3 == 0) {
                    this.a("mob.horse.gallop", ajm2.c() * 0.15f, ajm2.d());
                    if (n5 == 0 && this.Z.nextInt(10) == 0) {
                        this.a("mob.horse.breathe", ajm2.c() * 0.6f, ajm2.d());
                    }
                } else if (this.bP <= 5) {
                    this.a("mob.horse.wood", ajm2.c() * 0.15f, ajm2.d());
                }
            } else if (ajm2 == aji.f) {
                this.a("mob.horse.wood", ajm2.c() * 0.15f, ajm2.d());
            } else {
                this.a("mob.horse.soft", ajm2.c() * 0.15f, ajm2.d());
            }
        }
    }

    @Override
    protected void aD() {
        super.aD();
        this.bc().b(bv);
        this.a(yj.a).a(53.0);
        this.a(yj.d).a(0.225f);
    }

    @Override
    public int bB() {
        return 6;
    }

    public int cw() {
        return 100;
    }

    @Override
    protected float bf() {
        return 0.8f;
    }

    @Override
    public int q() {
        return 400;
    }

    private void cP() {
        this.bQ = null;
    }

    public void g(yz yz2) {
        if (!this.o.E && (this.l == null || this.l == yz2) && this.cc()) {
            this.bG.a(this.b_());
            yz2.a(this, this.bG);
        }
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.bx) {
            return super.a(yz2);
        }
        if (!this.cc() && this.cE()) {
            return false;
        }
        if (this.cc() && this.cb() && yz2.an()) {
            this.g(yz2);
            return true;
        }
        if (this.cg() && this.l != null) {
            return super.a(yz2);
        }
        if (add2 != null) {
            boolean bl2 = false;
            if (this.cB()) {
                int n2 = -1;
                if (add2.b() == ade.bX) {
                    n2 = 1;
                } else if (add2.b() == ade.bY) {
                    n2 = 2;
                } else if (add2.b() == ade.bZ) {
                    n2 = 3;
                }
                if (n2 >= 0) {
                    if (!this.cc()) {
                        this.cJ();
                        return true;
                    }
                    this.g(yz2);
                    return true;
                }
            }
            if (!bl2 && !this.cE()) {
                float f2 = 0.0f;
                int n3 = 0;
                int n4 = 0;
                if (add2.b() == ade.O) {
                    f2 = 2.0f;
                    n3 = 60;
                    n4 = 3;
                } else if (add2.b() == ade.aT) {
                    f2 = 1.0f;
                    n3 = 30;
                    n4 = 3;
                } else if (add2.b() == ade.P) {
                    f2 = 7.0f;
                    n3 = 180;
                    n4 = 3;
                } else if (aji.a(add2.b()) == ajn.cf) {
                    f2 = 20.0f;
                    n3 = 180;
                } else if (add2.b() == ade.e) {
                    f2 = 3.0f;
                    n3 = 60;
                    n4 = 3;
                } else if (add2.b() == ade.bK) {
                    f2 = 4.0f;
                    n3 = 60;
                    n4 = 5;
                    if (this.cc() && this.d() == 0) {
                        bl2 = true;
                        this.f(yz2);
                    }
                } else if (add2.b() == ade.ao) {
                    f2 = 10.0f;
                    n3 = 240;
                    n4 = 10;
                    if (this.cc() && this.d() == 0) {
                        bl2 = true;
                        this.f(yz2);
                    }
                }
                if (this.aS() < this.aY() && f2 > 0.0f) {
                    this.f(f2);
                    bl2 = true;
                }
                if (!this.cb() && n3 > 0) {
                    this.a(n3);
                    bl2 = true;
                }
                if (n4 > 0 && (bl2 || !this.cc()) && n4 < this.cw()) {
                    bl2 = true;
                    this.v(n4);
                }
                if (bl2) {
                    this.cL();
                }
            }
            if (!this.cc() && !bl2) {
                if (add2 != null && add2.a(yz2, (sv)this)) {
                    return true;
                }
                this.cJ();
                return true;
            }
            if (!bl2 && this.cC() && !this.ck() && add2.b() == adb.a(ajn.ae)) {
                this.l(true);
                this.a("mob.chickenplop", 1.0f, (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f);
                bl2 = true;
                this.cN();
            }
            if (!bl2 && this.cg() && !this.cu() && add2.b() == ade.av) {
                this.g(yz2);
                return true;
            }
            if (bl2) {
                if (!yz2.bE.d && --add2.b == 0) {
                    yz2.bm.a(yz2.bm.c, null);
                }
                return true;
            }
        }
        if (this.cg() && this.l == null) {
            if (add2 != null && add2.a(yz2, (sv)this)) {
                return true;
            }
            this.i(yz2);
            return true;
        }
        return super.a(yz2);
    }

    private void i(yz yz2) {
        yz2.y = this.y;
        yz2.z = this.z;
        this.o(false);
        this.p(false);
        if (!this.o.E) {
            yz2.a((sa)this);
        }
    }

    public boolean cB() {
        return this.bZ() == 0;
    }

    public boolean cC() {
        int n2 = this.bZ();
        return n2 == 2 || n2 == 1;
    }

    @Override
    protected boolean bh() {
        if (this.l != null && this.cu()) {
            return true;
        }
        return this.cm() || this.cn();
    }

    public boolean cE() {
        int n2 = this.bZ();
        return n2 == 3 || n2 == 4;
    }

    public boolean cF() {
        return this.cE() || this.bZ() == 2;
    }

    @Override
    public boolean c(add add2) {
        return false;
    }

    private void cR() {
        this.bp = 1;
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        if (!this.o.E) {
            this.cK();
        }
    }

    @Override
    public void e() {
        if (this.Z.nextInt(200) == 0) {
            this.cR();
        }
        super.e();
        if (!this.o.E) {
            wi wi2;
            if (this.Z.nextInt(900) == 0 && this.aA == 0) {
                this.f(1.0f);
            }
            if (!this.cm() && this.l == null && this.Z.nextInt(300) == 0 && this.o.a(qh.c(this.s), qh.c(this.t) - 1, qh.c(this.u)) == ajn.c) {
                this.o(true);
            }
            if (this.cm() && ++this.bD > 50) {
                this.bD = 0;
                this.o(false);
            }
            if (this.co() && !this.cb() && !this.cm() && (wi2 = this.a((sa)this, 16.0)) != null && this.f(wi2) > 4.0) {
                ayf ayf2 = this.o.a((sa)this, wi2, 16.0f, true, false, false, true);
                this.a(ayf2);
            }
        }
    }

    @Override
    public void h() {
        super.h();
        if (this.o.E && this.af.a()) {
            this.af.e();
            this.cP();
        }
        if (this.bE > 0 && ++this.bE > 30) {
            this.bE = 0;
            this.b(128, false);
        }
        if (!this.o.E && this.bF > 0 && ++this.bF > 20) {
            this.bF = 0;
            this.p(false);
        }
        if (this.bp > 0 && ++this.bp > 8) {
            this.bp = 0;
        }
        if (this.bq > 0) {
            ++this.bq;
            if (this.bq > 300) {
                this.bq = 0;
            }
        }
        this.bK = this.bJ;
        if (this.cm()) {
            this.bJ += (1.0f - this.bJ) * 0.4f + 0.05f;
            if (this.bJ > 1.0f) {
                this.bJ = 1.0f;
            }
        } else {
            this.bJ += (0.0f - this.bJ) * 0.4f - 0.05f;
            if (this.bJ < 0.0f) {
                this.bJ = 0.0f;
            }
        }
        this.bM = this.bL;
        if (this.cn()) {
            this.bJ = 0.0f;
            this.bK = 0.0f;
            this.bL += (1.0f - this.bL) * 0.4f + 0.05f;
            if (this.bL > 1.0f) {
                this.bL = 1.0f;
            }
        } else {
            this.bI = false;
            this.bL += (0.8f * this.bL * this.bL * this.bL - this.bL) * 0.6f - 0.05f;
            if (this.bL < 0.0f) {
                this.bL = 0.0f;
            }
        }
        this.bO = this.bN;
        if (this.x(128)) {
            this.bN += (1.0f - this.bN) * 0.7f + 0.05f;
            if (this.bN > 1.0f) {
                this.bN = 1.0f;
            }
        } else {
            this.bN += (0.0f - this.bN) * 0.7f - 0.05f;
            if (this.bN < 0.0f) {
                this.bN = 0.0f;
            }
        }
    }

    private void cS() {
        if (!this.o.E) {
            this.bE = 1;
            this.b(128, true);
        }
    }

    private boolean cT() {
        return this.l == null && this.m == null && this.cc() && this.cb() && !this.cF() && this.aS() >= this.aY();
    }

    @Override
    public void e(boolean bl2) {
        this.b(32, bl2);
    }

    public void o(boolean bl2) {
        this.e(bl2);
    }

    public void p(boolean bl2) {
        if (bl2) {
            this.o(false);
        }
        this.b(64, bl2);
    }

    private void cU() {
        if (!this.o.E) {
            this.bF = 1;
            this.p(true);
        }
    }

    public void cJ() {
        this.cU();
        String string = this.cv();
        if (string != null) {
            this.a(string, this.bf(), this.bg());
        }
    }

    public void cK() {
        this.a((sa)this, this.bG);
        this.cs();
    }

    private void a(sa sa2, zt zt2) {
        if (zt2 == null || this.o.E) {
            return;
        }
        for (int i2 = 0; i2 < zt2.a(); ++i2) {
            add add2 = zt2.a(i2);
            if (add2 == null) continue;
            this.a(add2, 0.0f);
        }
    }

    public boolean h(yz yz2) {
        this.b(yz2.aB().toString());
        this.i(true);
        return true;
    }

    @Override
    public void e(float f2, float f3) {
        if (this.l == null || !(this.l instanceof sv) || !this.cu()) {
            this.W = 0.5f;
            this.aQ = 0.02f;
            super.e(f2, f3);
            return;
        }
        this.A = this.y = this.l.y;
        this.z = this.l.z * 0.5f;
        this.b(this.y, this.z);
        this.aO = this.aM = this.y;
        f2 = ((sv)this.l).bd * 0.5f;
        f3 = ((sv)this.l).be;
        if (f3 <= 0.0f) {
            f3 *= 0.25f;
            this.bP = 0;
        }
        if (this.D && this.bt == 0.0f && this.cn() && !this.bI) {
            f2 = 0.0f;
            f3 = 0.0f;
        }
        if (this.bt > 0.0f && !this.cj() && this.D) {
            this.w = this.ct() * (double)this.bt;
            if (this.a(rv.j)) {
                this.w += (double)((float)(this.b(rv.j).c() + 1) * 0.1f);
            }
            this.j(true);
            this.al = true;
            if (f3 > 0.0f) {
                float f4 = qh.a(this.y * (float)Math.PI / 180.0f);
                float f5 = qh.b(this.y * (float)Math.PI / 180.0f);
                this.v += (double)(-0.4f * f4 * this.bt);
                this.x += (double)(0.4f * f5 * this.bt);
                this.a("mob.horse.jump", 0.4f, 1.0f);
            }
            this.bt = 0.0f;
        }
        this.W = 1.0f;
        this.aQ = this.bl() * 0.1f;
        if (!this.o.E) {
            this.i((float)this.a(yj.d).e());
            super.e(f2, f3);
        }
        if (this.D) {
            this.bt = 0.0f;
            this.j(false);
        }
        this.aE = this.aF;
        double d2 = this.s - this.p;
        double d3 = this.u - this.r;
        float f6 = qh.a(d2 * d2 + d3 * d3) * 4.0f;
        if (f6 > 1.0f) {
            f6 = 1.0f;
        }
        this.aF += (f6 - this.aF) * 0.4f;
        this.aG += this.aF;
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("EatingHaystack", this.cm());
        dh2.a("ChestedHorse", this.ck());
        dh2.a("HasReproduced", this.cp());
        dh2.a("Bred", this.co());
        dh2.a("Type", this.bZ());
        dh2.a("Variant", this.ca());
        dh2.a("Temper", this.cq());
        dh2.a("Tame", this.cc());
        dh2.a("OwnerUUID", this.ch());
        if (this.ck()) {
            dq dq2 = new dq();
            for (int i2 = 2; i2 < this.bG.a(); ++i2) {
                add add2 = this.bG.a(i2);
                if (add2 == null) continue;
                dh dh3 = new dh();
                dh3.a("Slot", (byte)i2);
                add2.b(dh3);
                dq2.a(dh3);
            }
            dh2.a("Items", dq2);
        }
        if (this.bG.a(1) != null) {
            dh2.a("ArmorItem", this.bG.a(1).b(new dh()));
        }
        if (this.bG.a(0) != null) {
            dh2.a("SaddleItem", this.bG.a(0).b(new dh()));
        }
    }

    @Override
    public void a(dh dh2) {
        Object object;
        ti ti2;
        super.a(dh2);
        this.o(dh2.n("EatingHaystack"));
        this.k(dh2.n("Bred"));
        this.l(dh2.n("ChestedHorse"));
        this.m(dh2.n("HasReproduced"));
        this.s(dh2.f("Type"));
        this.t(dh2.f("Variant"));
        this.u(dh2.f("Temper"));
        this.i(dh2.n("Tame"));
        if (dh2.b("OwnerUUID", 8)) {
            this.b(dh2.j("OwnerUUID"));
        }
        if ((ti2 = this.bc().a("Speed")) != null) {
            this.a(yj.d).a(ti2.b() * 0.25);
        }
        if (this.ck()) {
            object = dh2.c("Items", 10);
            this.cN();
            for (int i2 = 0; i2 < ((dq)object).c(); ++i2) {
                dh dh3 = ((dq)object).b(i2);
                int n2 = dh3.d("Slot") & 0xFF;
                if (n2 < 2 || n2 >= this.bG.a()) continue;
                this.bG.a(n2, add.a(dh3));
            }
        }
        if (dh2.b("ArmorItem", 10) && (object = add.a(dh2.m("ArmorItem"))) != null && wi.a(((add)object).b())) {
            this.bG.a(1, (add)object);
        }
        if (dh2.b("SaddleItem", 10)) {
            object = add.a(dh2.m("SaddleItem"));
            if (object != null && ((add)object).b() == ade.av) {
                this.bG.a(0, (add)object);
            }
        } else if (dh2.n("Saddle")) {
            this.bG.a(0, new add(ade.av));
        }
        this.cO();
    }

    @Override
    public boolean a(wf wf2) {
        int n2;
        if (wf2 == this) {
            return false;
        }
        if (wf2.getClass() != this.getClass()) {
            return false;
        }
        wi wi2 = (wi)wf2;
        if (!this.cT() || !wi2.cT()) {
            return false;
        }
        int n3 = this.bZ();
        return n3 == (n2 = wi2.bZ()) || n3 == 0 && n2 == 1 || n3 == 1 && n2 == 0;
    }

    @Override
    public rx a(rx rx2) {
        wi wi2 = (wi)rx2;
        wi wi3 = new wi(this.o);
        int n2 = this.bZ();
        int n3 = wi2.bZ();
        int n4 = 0;
        if (n2 == n3) {
            n4 = n2;
        } else if (n2 == 0 && n3 == 1 || n2 == 1 && n3 == 0) {
            n4 = 2;
        }
        if (n4 == 0) {
            int n5 = this.Z.nextInt(9);
            int n6 = n5 < 4 ? this.ca() & 0xFF : (n5 < 8 ? wi2.ca() & 0xFF : this.Z.nextInt(7));
            int n7 = this.Z.nextInt(5);
            n6 = n7 < 2 ? (n6 |= this.ca() & 0xFF00) : (n7 < 4 ? (n6 |= wi2.ca() & 0xFF00) : (n6 |= this.Z.nextInt(5) << 8 & 0xFF00));
            wi3.t(n6);
        }
        wi3.s(n4);
        double d2 = this.a(yj.a).b() + rx2.a(yj.a).b() + (double)this.cV();
        wi3.a(yj.a).a(d2 / 3.0);
        double d3 = this.a(bv).b() + rx2.a(bv).b() + this.cW();
        wi3.a(bv).a(d3 / 3.0);
        double d4 = this.a(yj.d).b() + rx2.a(yj.d).b() + this.cX();
        wi3.a(yj.d).a(d4 / 3.0);
        return wi3;
    }

    @Override
    public sy a(sy sy2) {
        sy2 = super.a(sy2);
        int n2 = 0;
        int n3 = 0;
        if (sy2 instanceof wk) {
            n2 = ((wk)sy2).a;
            n3 = ((wk)sy2).b & 0xFF | this.Z.nextInt(5) << 8;
        } else {
            if (this.Z.nextInt(10) == 0) {
                n2 = 1;
            } else {
                int n4 = this.Z.nextInt(7);
                int n5 = this.Z.nextInt(5);
                n2 = 0;
                n3 = n4 | n5 << 8;
            }
            sy2 = new wk(n2, n3);
        }
        this.s(n2);
        this.t(n3);
        if (this.Z.nextInt(5) == 0) {
            this.c(-24000);
        }
        if (n2 == 4 || n2 == 3) {
            this.a(yj.a).a(15.0);
            this.a(yj.d).a(0.2f);
        } else {
            this.a(yj.a).a(this.cV());
            if (n2 == 0) {
                this.a(yj.d).a(this.cX());
            } else {
                this.a(yj.d).a(0.175f);
            }
        }
        if (n2 == 2 || n2 == 1) {
            this.a(bv).a(0.5);
        } else {
            this.a(bv).a(this.cW());
        }
        this.g(this.aY());
        return sy2;
    }

    @Override
    protected boolean bk() {
        return true;
    }

    public void w(int n2) {
        if (this.cu()) {
            if (n2 < 0) {
                n2 = 0;
            } else {
                this.bI = true;
                this.cU();
            }
            this.bt = n2 >= 90 ? 1.0f : 0.4f + 0.4f * (float)n2 / 90.0f;
        }
    }

    @Override
    public void ac() {
        super.ac();
        if (this.bM > 0.0f) {
            float f2 = qh.a(this.aM * (float)Math.PI / 180.0f);
            float f3 = qh.b(this.aM * (float)Math.PI / 180.0f);
            float f4 = 0.7f * this.bM;
            float f5 = 0.15f * this.bM;
            this.l.b(this.s + (double)(f4 * f2), this.t + this.ae() + this.l.ad() + (double)f5, this.u - (double)(f4 * f3));
            if (this.l instanceof sv) {
                ((sv)this.l).aM = this.aM;
            }
        }
    }

    private float cV() {
        return 15.0f + (float)this.Z.nextInt(8) + (float)this.Z.nextInt(9);
    }

    private double cW() {
        return (double)0.4f + this.Z.nextDouble() * 0.2 + this.Z.nextDouble() * 0.2 + this.Z.nextDouble() * 0.2;
    }

    private double cX() {
        return ((double)0.45f + this.Z.nextDouble() * 0.3 + this.Z.nextDouble() * 0.3 + this.Z.nextDouble() * 0.3) * 0.25;
    }

    public static boolean a(adb adb2) {
        return adb2 == ade.bX || adb2 == ade.bY || adb2 == ade.bZ;
    }

    @Override
    public boolean h_() {
        return false;
    }
}

