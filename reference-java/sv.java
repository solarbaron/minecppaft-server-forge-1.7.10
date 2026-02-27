/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.UUID;

public abstract class sv
extends sa {
    private static final UUID b = UUID.fromString("662A6B8D-DA3E-4C1C-8813-96EA6097278D");
    private static final tj c = new tj(b, "Sprinting speed boost", 0.3f, 2).a(false);
    private tl d;
    private final rn e = new rn(this);
    private final HashMap f = new HashMap();
    private final add[] g = new add[5];
    public boolean at;
    public int au;
    public int av;
    public float aw;
    public int ax;
    public int ay;
    public float az;
    public int aA;
    public int aB;
    public float aC;
    public float aD;
    public float aE;
    public float aF;
    public float aG;
    public int aH = 20;
    public float aI;
    public float aJ;
    public float aK;
    public float aL;
    public float aM;
    public float aN;
    public float aO;
    public float aP;
    public float aQ = 0.02f;
    protected yz aR;
    protected int aS;
    protected boolean aT;
    protected int aU;
    protected float aV;
    protected float aW;
    protected float aX;
    protected float aY;
    protected float aZ;
    protected int ba;
    protected float bb;
    protected boolean bc;
    public float bd;
    public float be;
    protected float bf;
    protected int bg;
    protected double bh;
    protected double bi;
    protected double bj;
    protected double bk;
    protected double bl;
    private boolean h = true;
    private sv i;
    private int bm;
    private sv bn;
    private int bo;
    private float bp;
    private int bq;
    private float br;

    public sv(ahb ahb2) {
        super(ahb2);
        this.aD();
        this.g(this.aY());
        this.k = true;
        this.aL = (float)(Math.random() + 1.0) * 0.01f;
        this.b(this.s, this.t, this.u);
        this.aK = (float)Math.random() * 12398.0f;
        this.aO = this.y = (float)(Math.random() * 3.1415927410125732 * 2.0);
        this.W = 0.5f;
    }

    @Override
    protected void c() {
        this.af.a(7, (Object)0);
        this.af.a(8, (Object)0);
        this.af.a(9, (Object)0);
        this.af.a(6, Float.valueOf(1.0f));
    }

    protected void aD() {
        this.bc().b(yj.a);
        this.bc().b(yj.c);
        this.bc().b(yj.d);
        if (!this.bk()) {
            this.a(yj.d).a(0.1f);
        }
    }

    @Override
    protected void a(double d2, boolean bl2) {
        if (!this.M()) {
            this.N();
        }
        if (bl2 && this.R > 0.0f) {
            int n2;
            int n3;
            int n4 = qh.c(this.s);
            aji aji2 = this.o.a(n4, n3 = qh.c(this.t - (double)0.2f - (double)this.L), n2 = qh.c(this.u));
            if (aji2.o() == awt.a) {
                int n5 = this.o.a(n4, n3 - 1, n2).b();
                if (n5 == 11 || n5 == 32 || n5 == 21) {
                    aji2 = this.o.a(n4, n3 - 1, n2);
                }
            } else if (!this.o.E && this.R > 3.0f) {
                this.o.c(2006, n4, n3, n2, qh.f(this.R - 3.0f));
            }
            aji2.a(this.o, n4, n3, n2, (sa)this, this.R);
        }
        super.a(d2, bl2);
    }

    public boolean aE() {
        return false;
    }

    @Override
    public void C() {
        boolean bl2;
        this.aC = this.aD;
        super.C();
        this.o.C.a("livingEntityBaseTick");
        if (this.Z() && this.aa()) {
            this.a(ro.d, 1.0f);
        }
        if (this.K() || this.o.E) {
            this.F();
        }
        boolean bl3 = bl2 = this instanceof yz && ((yz)this).bE.a;
        if (this.Z() && this.a(awt.h)) {
            if (!(this.aE() || this.k(rv.o.H) || bl2)) {
                this.h(this.j(this.ar()));
                if (this.ar() == -20) {
                    this.h(0);
                    for (int i2 = 0; i2 < 8; ++i2) {
                        float f2 = this.Z.nextFloat() - this.Z.nextFloat();
                        float f3 = this.Z.nextFloat() - this.Z.nextFloat();
                        float f4 = this.Z.nextFloat() - this.Z.nextFloat();
                        this.o.a("bubble", this.s + (double)f2, this.t + (double)f3, this.u + (double)f4, this.v, this.w, this.x);
                    }
                    this.a(ro.e, 2.0f);
                }
            }
            if (!this.o.E && this.am() && this.m instanceof sv) {
                this.a((sa)null);
            }
        } else {
            this.h(300);
        }
        if (this.Z() && this.L()) {
            this.F();
        }
        this.aI = this.aJ;
        if (this.aB > 0) {
            --this.aB;
        }
        if (this.ax > 0) {
            --this.ax;
        }
        if (this.ad > 0 && !(this instanceof mw)) {
            --this.ad;
        }
        if (this.aS() <= 0.0f) {
            this.aF();
        }
        if (this.aS > 0) {
            --this.aS;
        } else {
            this.aR = null;
        }
        if (this.bn != null && !this.bn.Z()) {
            this.bn = null;
        }
        if (this.i != null) {
            if (!this.i.Z()) {
                this.b((sv)null);
            } else if (this.aa - this.bm > 100) {
                this.b((sv)null);
            }
        }
        this.aO();
        this.aY = this.aX;
        this.aN = this.aM;
        this.aP = this.aO;
        this.A = this.y;
        this.B = this.z;
        this.o.C.b();
    }

    public boolean f() {
        return false;
    }

    protected void aF() {
        ++this.aA;
        if (this.aA == 20) {
            int n2;
            if (!this.o.E && (this.aS > 0 || this.aH()) && this.aG() && this.o.O().b("doMobLoot")) {
                int n3;
                for (n2 = this.e(this.aR); n2 > 0; n2 -= n3) {
                    n3 = sq.a(n2);
                    this.o.d(new sq(this.o, this.s, this.t, this.u, n3));
                }
            }
            this.B();
            for (n2 = 0; n2 < 20; ++n2) {
                double d2 = this.Z.nextGaussian() * 0.02;
                double d3 = this.Z.nextGaussian() * 0.02;
                double d4 = this.Z.nextGaussian() * 0.02;
                this.o.a("explode", this.s + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, this.t + (double)(this.Z.nextFloat() * this.N), this.u + (double)(this.Z.nextFloat() * this.M * 2.0f) - (double)this.M, d2, d3, d4);
            }
        }
    }

    protected boolean aG() {
        return !this.f();
    }

    protected int j(int n2) {
        int n3 = afv.b(this);
        if (n3 > 0 && this.Z.nextInt(n3 + 1) > 0) {
            return n2;
        }
        return n2 - 1;
    }

    protected int e(yz yz2) {
        return 0;
    }

    protected boolean aH() {
        return false;
    }

    public Random aI() {
        return this.Z;
    }

    public sv aJ() {
        return this.i;
    }

    public int aK() {
        return this.bm;
    }

    public void b(sv sv2) {
        this.i = sv2;
        this.bm = this.aa;
    }

    public sv aL() {
        return this.bn;
    }

    public int aM() {
        return this.bo;
    }

    public void l(sa sa2) {
        this.bn = sa2 instanceof sv ? (sv)sa2 : null;
        this.bo = this.aa;
    }

    public int aN() {
        return this.aU;
    }

    @Override
    public void b(dh dh2) {
        dh2.a("HealF", this.aS());
        dh2.a("Health", (short)Math.ceil(this.aS()));
        dh2.a("HurtTime", (short)this.ax);
        dh2.a("DeathTime", (short)this.aA);
        dh2.a("AttackTime", (short)this.aB);
        dh2.a("AbsorptionAmount", this.bs());
        for (add add2 : this.ak()) {
            if (add2 == null) continue;
            this.d.a(add2.D());
        }
        dh2.a("Attributes", yj.a(this.bc()));
        for (add add2 : this.ak()) {
            if (add2 == null) continue;
            this.d.b(add2.D());
        }
        if (!this.f.isEmpty()) {
            dq dq2 = new dq();
            for (rw rw2 : this.f.values()) {
                dq2.a(rw2.a(new dh()));
            }
            dh2.a("ActiveEffects", dq2);
        }
    }

    @Override
    public void a(dh dh2) {
        dy dy2;
        this.m(dh2.h("AbsorptionAmount"));
        if (dh2.b("Attributes", 9) && this.o != null && !this.o.E) {
            yj.a(this.bc(), dh2.c("Attributes", 10));
        }
        if (dh2.b("ActiveEffects", 9)) {
            dy2 = dh2.c("ActiveEffects", 10);
            for (int i2 = 0; i2 < ((dq)dy2).c(); ++i2) {
                dh dh3 = ((dq)dy2).b(i2);
                rw rw2 = rw.b(dh3);
                if (rw2 == null) continue;
                this.f.put(rw2.a(), rw2);
            }
        }
        if (dh2.b("HealF", 99)) {
            this.g(dh2.h("HealF"));
        } else {
            dy2 = dh2.a("Health");
            if (dy2 == null) {
                this.g(this.aY());
            } else if (dy2.a() == 5) {
                this.g(((dm)dy2).h());
            } else if (dy2.a() == 2) {
                this.g((float)((dw)dy2).e());
            }
        }
        this.ax = dh2.e("HurtTime");
        this.aA = dh2.e("DeathTime");
        this.aB = dh2.e("AttackTime");
    }

    protected void aO() {
        boolean bl2;
        Iterator iterator = this.f.keySet().iterator();
        while (iterator.hasNext()) {
            Integer n2 = (Integer)iterator.next();
            rw rw2 = (rw)this.f.get(n2);
            if (!rw2.a(this)) {
                if (this.o.E) continue;
                iterator.remove();
                this.b(rw2);
                continue;
            }
            if (rw2.b() % 600 != 0) continue;
            this.a(rw2, false);
        }
        if (this.h) {
            if (!this.o.E) {
                if (this.f.isEmpty()) {
                    this.af.b(8, (byte)0);
                    this.af.b(7, 0);
                    this.d(false);
                } else {
                    int n3 = aen.a(this.f.values());
                    this.af.b(8, aen.b(this.f.values()) ? (byte)1 : 0);
                    this.af.b(7, n3);
                    this.d(this.k(rv.p.H));
                }
            }
            this.h = false;
        }
        int n4 = this.af.c(7);
        boolean bl3 = bl2 = this.af.a(8) > 0;
        if (n4 > 0) {
            boolean bl4 = false;
            if (!this.ap()) {
                bl4 = this.Z.nextBoolean();
            } else {
                boolean bl5 = bl4 = this.Z.nextInt(15) == 0;
            }
            if (bl2) {
                bl4 &= this.Z.nextInt(5) == 0;
            }
            if (bl4 && n4 > 0) {
                double d2 = (double)(n4 >> 16 & 0xFF) / 255.0;
                double d3 = (double)(n4 >> 8 & 0xFF) / 255.0;
                double d4 = (double)(n4 >> 0 & 0xFF) / 255.0;
                this.o.a(bl2 ? "mobSpellAmbient" : "mobSpell", this.s + (this.Z.nextDouble() - 0.5) * (double)this.M, this.t + this.Z.nextDouble() * (double)this.N - (double)this.L, this.u + (this.Z.nextDouble() - 0.5) * (double)this.M, d2, d3, d4);
            }
        }
    }

    public void aP() {
        Iterator iterator = this.f.keySet().iterator();
        while (iterator.hasNext()) {
            Integer n2 = (Integer)iterator.next();
            rw rw2 = (rw)this.f.get(n2);
            if (this.o.E) continue;
            iterator.remove();
            this.b(rw2);
        }
    }

    public Collection aQ() {
        return this.f.values();
    }

    public boolean k(int n2) {
        return this.f.containsKey(n2);
    }

    public boolean a(rv rv2) {
        return this.f.containsKey(rv2.H);
    }

    public rw b(rv rv2) {
        return (rw)this.f.get(rv2.H);
    }

    public void c(rw rw2) {
        if (!this.d(rw2)) {
            return;
        }
        if (this.f.containsKey(rw2.a())) {
            ((rw)this.f.get(rw2.a())).a(rw2);
            this.a((rw)this.f.get(rw2.a()), true);
        } else {
            this.f.put(rw2.a(), rw2);
            this.a(rw2);
        }
    }

    public boolean d(rw rw2) {
        int n2;
        return this.bd() != sz.b || (n2 = rw2.a()) != rv.l.H && n2 != rv.u.H;
    }

    public boolean aR() {
        return this.bd() == sz.b;
    }

    public void m(int n2) {
        rw rw2 = (rw)this.f.remove(n2);
        if (rw2 != null) {
            this.b(rw2);
        }
    }

    protected void a(rw rw2) {
        this.h = true;
        if (!this.o.E) {
            rv.a[rw2.a()].b(this, this.bc(), rw2.c());
        }
    }

    protected void a(rw rw2, boolean bl2) {
        this.h = true;
        if (bl2 && !this.o.E) {
            rv.a[rw2.a()].a(this, this.bc(), rw2.c());
            rv.a[rw2.a()].b(this, this.bc(), rw2.c());
        }
    }

    protected void b(rw rw2) {
        this.h = true;
        if (!this.o.E) {
            rv.a[rw2.a()].a(this, this.bc(), rw2.c());
        }
    }

    public void f(float f2) {
        float f3 = this.aS();
        if (f3 > 0.0f) {
            this.g(f3 + f2);
        }
    }

    public final float aS() {
        return this.af.d(6);
    }

    public void g(float f2) {
        this.af.b(6, Float.valueOf(qh.a(f2, 0.0f, this.aY())));
    }

    @Override
    public boolean a(ro ro2, float f2) {
        Object object;
        if (this.aw()) {
            return false;
        }
        if (this.o.E) {
            return false;
        }
        this.aU = 0;
        if (this.aS() <= 0.0f) {
            return false;
        }
        if (ro2.o() && this.a(rv.n)) {
            return false;
        }
        if ((ro2 == ro.m || ro2 == ro.n) && this.q(4) != null) {
            this.q(4).a((int)(f2 * 4.0f + this.Z.nextFloat() * f2 * 2.0f), this);
            f2 *= 0.75f;
        }
        this.aF = 1.5f;
        boolean bl2 = true;
        if ((float)this.ad > (float)this.aH / 2.0f) {
            if (f2 <= this.bb) {
                return false;
            }
            this.d(ro2, f2 - this.bb);
            this.bb = f2;
            bl2 = false;
        } else {
            this.bb = f2;
            this.aw = this.aS();
            this.ad = this.aH;
            this.d(ro2, f2);
            this.ay = 10;
            this.ax = 10;
        }
        this.az = 0.0f;
        sa sa2 = ro2.j();
        if (sa2 != null) {
            if (sa2 instanceof sv) {
                this.b((sv)sa2);
            }
            if (sa2 instanceof yz) {
                this.aS = 100;
                this.aR = (yz)sa2;
            } else if (sa2 instanceof wv && ((tg)(object = (wv)sa2)).bZ()) {
                this.aS = 100;
                this.aR = null;
            }
        }
        if (bl2) {
            this.o.a((sa)this, (byte)2);
            if (ro2 != ro.e) {
                this.Q();
            }
            if (sa2 != null) {
                double d2 = sa2.s - this.s;
                double d3 = sa2.u - this.u;
                while (d2 * d2 + d3 * d3 < 1.0E-4) {
                    d2 = (Math.random() - Math.random()) * 0.01;
                    d3 = (Math.random() - Math.random()) * 0.01;
                }
                this.az = (float)(Math.atan2(d3, d2) * 180.0 / 3.1415927410125732) - this.y;
                this.a(sa2, f2, d2, d3);
            } else {
                this.az = (int)(Math.random() * 2.0) * 180;
            }
        }
        if (this.aS() <= 0.0f) {
            object = this.aU();
            if (bl2 && object != null) {
                this.a((String)object, this.bf(), this.bg());
            }
            this.a(ro2);
        } else {
            object = this.aT();
            if (bl2 && object != null) {
                this.a((String)object, this.bf(), this.bg());
            }
        }
        return true;
    }

    public void a(add add2) {
        this.a("random.break", 0.8f, 0.8f + this.o.s.nextFloat() * 0.4f);
        for (int i2 = 0; i2 < 5; ++i2) {
            azw azw2 = azw.a(((double)this.Z.nextFloat() - 0.5) * 0.1, Math.random() * 0.1 + 0.1, 0.0);
            azw2.a(-this.z * (float)Math.PI / 180.0f);
            azw2.b(-this.y * (float)Math.PI / 180.0f);
            azw azw3 = azw.a(((double)this.Z.nextFloat() - 0.5) * 0.3, (double)(-this.Z.nextFloat()) * 0.6 - 0.3, 0.6);
            azw3.a(-this.z * (float)Math.PI / 180.0f);
            azw3.b(-this.y * (float)Math.PI / 180.0f);
            azw3 = azw3.c(this.s, this.t + (double)this.g(), this.u);
            this.o.a("iconcrack_" + adb.b(add2.b()), azw3.a, azw3.b, azw3.c, azw2.a, azw2.b + 0.05, azw2.c);
        }
    }

    public void a(ro ro2) {
        sa sa2 = ro2.j();
        sv sv2 = this.aX();
        if (this.ba >= 0 && sv2 != null) {
            sv2.b(this, this.ba);
        }
        if (sa2 != null) {
            sa2.a(this);
        }
        this.aT = true;
        this.aW().g();
        if (!this.o.E) {
            int n2 = 0;
            if (sa2 instanceof yz) {
                n2 = afv.i((sv)sa2);
            }
            if (this.aG() && this.o.O().b("doMobLoot")) {
                int n3;
                this.b(this.aS > 0, n2);
                this.a(this.aS > 0, n2);
                if (this.aS > 0 && (n3 = this.Z.nextInt(200) - n2) < 5) {
                    this.n(n3 <= 0 ? 1 : 0);
                }
            }
        }
        this.o.a((sa)this, (byte)3);
    }

    protected void a(boolean bl2, int n2) {
    }

    public void a(sa sa2, float f2, double d2, double d3) {
        if (this.Z.nextDouble() < this.a(yj.c).e()) {
            return;
        }
        this.al = true;
        float f3 = qh.a(d2 * d2 + d3 * d3);
        float f4 = 0.4f;
        this.v /= 2.0;
        this.w /= 2.0;
        this.x /= 2.0;
        this.v -= d2 / (double)f3 * (double)f4;
        this.w += (double)f4;
        this.x -= d3 / (double)f3 * (double)f4;
        if (this.w > (double)0.4f) {
            this.w = 0.4f;
        }
    }

    protected String aT() {
        return "game.neutral.hurt";
    }

    protected String aU() {
        return "game.neutral.die";
    }

    protected void n(int n2) {
    }

    protected void b(boolean bl2, int n2) {
    }

    public boolean h_() {
        int n2;
        int n3;
        int n4 = qh.c(this.s);
        aji aji2 = this.o.a(n4, n3 = qh.c(this.C.b), n2 = qh.c(this.u));
        return aji2 == ajn.ap || aji2 == ajn.bd;
    }

    @Override
    public boolean Z() {
        return !this.K && this.aS() > 0.0f;
    }

    @Override
    protected void b(float f2) {
        super.b(f2);
        rw rw2 = this.b(rv.j);
        float f3 = rw2 != null ? (float)(rw2.c() + 1) : 0.0f;
        int n2 = qh.f(f2 - 3.0f - f3);
        if (n2 > 0) {
            this.a(this.o(n2), 1.0f, 1.0f);
            this.a(ro.h, (float)n2);
            int n3 = qh.c(this.s);
            int n4 = qh.c(this.t - (double)0.2f - (double)this.L);
            int n5 = qh.c(this.u);
            aji aji2 = this.o.a(n3, n4, n5);
            if (aji2.o() != awt.a) {
                ajm ajm2 = aji2.H;
                this.a(ajm2.e(), ajm2.c() * 0.5f, ajm2.d() * 0.75f);
            }
        }
    }

    protected String o(int n2) {
        if (n2 > 4) {
            return "game.neutral.hurt.fall.big";
        }
        return "game.neutral.hurt.fall.small";
    }

    public int aV() {
        int n2 = 0;
        for (add add2 : this.ak()) {
            if (add2 == null || !(add2.b() instanceof abb)) continue;
            int n3 = ((abb)add2.b()).c;
            n2 += n3;
        }
        return n2;
    }

    protected void h(float f2) {
    }

    protected float b(ro ro2, float f2) {
        if (!ro2.e()) {
            int n2 = 25 - this.aV();
            float f3 = f2 * (float)n2;
            this.h(f2);
            f2 = f3 / 25.0f;
        }
        return f2;
    }

    protected float c(ro ro2, float f2) {
        float f3;
        int n2;
        int n3;
        if (ro2.h()) {
            return f2;
        }
        if (this instanceof yq) {
            // empty if block
        }
        if (this.a(rv.m) && ro2 != ro.i) {
            n3 = (this.b(rv.m).c() + 1) * 5;
            n2 = 25 - n3;
            f3 = f2 * (float)n2;
            f2 = f3 / 25.0f;
        }
        if (f2 <= 0.0f) {
            return 0.0f;
        }
        n3 = afv.a(this.ak(), ro2);
        if (n3 > 20) {
            n3 = 20;
        }
        if (n3 > 0 && n3 <= 20) {
            n2 = 25 - n3;
            f3 = f2 * (float)n2;
            f2 = f3 / 25.0f;
        }
        return f2;
    }

    protected void d(ro ro2, float f2) {
        if (this.aw()) {
            return;
        }
        f2 = this.b(ro2, f2);
        float f3 = f2 = this.c(ro2, f2);
        f2 = Math.max(f2 - this.bs(), 0.0f);
        this.m(this.bs() - (f3 - f2));
        if (f2 == 0.0f) {
            return;
        }
        float f4 = this.aS();
        this.g(f4 - f2);
        this.aW().a(ro2, f4, f2);
        this.m(this.bs() - f2);
    }

    public rn aW() {
        return this.e;
    }

    public sv aX() {
        if (this.e.c() != null) {
            return this.e.c();
        }
        if (this.aR != null) {
            return this.aR;
        }
        if (this.i != null) {
            return this.i;
        }
        return null;
    }

    public final float aY() {
        return (float)this.a(yj.a).e();
    }

    public final int aZ() {
        return this.af.a(9);
    }

    public final void p(int n2) {
        this.af.b(9, (byte)n2);
    }

    private int j() {
        if (this.a(rv.e)) {
            return 6 - (1 + this.b(rv.e).c()) * 1;
        }
        if (this.a(rv.f)) {
            return 6 + (1 + this.b(rv.f).c()) * 2;
        }
        return 6;
    }

    public void ba() {
        if (!this.at || this.au >= this.j() / 2 || this.au < 0) {
            this.au = -1;
            this.at = true;
            if (this.o instanceof mt) {
                ((mt)this.o).r().a(this, new gc(this, 0));
            }
        }
    }

    @Override
    protected void G() {
        this.a(ro.i, 4.0f);
    }

    protected void bb() {
        int n2 = this.j();
        if (this.at) {
            ++this.au;
            if (this.au >= n2) {
                this.au = 0;
                this.at = false;
            }
        } else {
            this.au = 0;
        }
        this.aD = (float)this.au / (float)n2;
    }

    public ti a(th th2) {
        return this.bc().a(th2);
    }

    public tl bc() {
        if (this.d == null) {
            this.d = new tq();
        }
        return this.d;
    }

    public sz bd() {
        return sz.a;
    }

    public abstract add be();

    public abstract add q(int var1);

    @Override
    public abstract void c(int var1, add var2);

    @Override
    public void c(boolean bl2) {
        super.c(bl2);
        ti ti2 = this.a(yj.d);
        if (ti2.a(b) != null) {
            ti2.b(c);
        }
        if (bl2) {
            ti2.a(c);
        }
    }

    @Override
    public abstract add[] ak();

    protected float bf() {
        return 1.0f;
    }

    protected float bg() {
        if (this.f()) {
            return (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.5f;
        }
        return (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f;
    }

    protected boolean bh() {
        return this.aS() <= 0.0f;
    }

    public void a(double d2, double d3, double d4) {
        this.b(d2, d3, d4, this.y, this.z);
    }

    public void m(sa sa2) {
        double d2 = sa2.s;
        double d3 = sa2.C.b + (double)sa2.N;
        double d4 = sa2.u;
        int n2 = 1;
        for (int i2 = -n2; i2 <= n2; ++i2) {
            for (int i3 = -n2; i3 < n2; ++i3) {
                if (i2 == 0 && i3 == 0) continue;
                int n3 = (int)(this.s + (double)i2);
                int n4 = (int)(this.u + (double)i3);
                azt azt2 = this.C.c(i2, 1.0, i3);
                if (!this.o.a(azt2).isEmpty()) continue;
                if (ahb.a(this.o, n3, (int)this.t, n4)) {
                    this.a(this.s + (double)i2, this.t + 1.0, this.u + (double)i3);
                    return;
                }
                if (!ahb.a(this.o, n3, (int)this.t - 1, n4) && this.o.a(n3, (int)this.t - 1, n4).o() != awt.h) continue;
                d2 = this.s + (double)i2;
                d3 = this.t + 1.0;
                d4 = this.u + (double)i3;
            }
        }
        this.a(d2, d3, d4);
    }

    protected void bj() {
        this.w = 0.42f;
        if (this.a(rv.j)) {
            this.w += (double)((float)(this.b(rv.j).c() + 1) * 0.1f);
        }
        if (this.ao()) {
            float f2 = this.y * ((float)Math.PI / 180);
            this.v -= (double)(qh.a(f2) * 0.2f);
            this.x += (double)(qh.b(f2) * 0.2f);
        }
        this.al = true;
    }

    public void e(float f2, float f3) {
        if (!(!this.M() || this instanceof yz && ((yz)this).bE.b)) {
            double d2 = this.t;
            this.a(f2, f3, this.bk() ? 0.04f : 0.02f);
            this.d(this.v, this.w, this.x);
            this.v *= (double)0.8f;
            this.w *= (double)0.8f;
            this.x *= (double)0.8f;
            this.w -= 0.02;
            if (this.E && this.c(this.v, this.w + (double)0.6f - this.t + d2, this.x)) {
                this.w = 0.3f;
            }
        } else if (!(!this.P() || this instanceof yz && ((yz)this).bE.b)) {
            double d3 = this.t;
            this.a(f2, f3, 0.02f);
            this.d(this.v, this.w, this.x);
            this.v *= 0.5;
            this.w *= 0.5;
            this.x *= 0.5;
            this.w -= 0.02;
            if (this.E && this.c(this.v, this.w + (double)0.6f - this.t + d3, this.x)) {
                this.w = 0.3f;
            }
        } else {
            float f4 = 0.91f;
            if (this.D) {
                f4 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.91f;
            }
            float f5 = 0.16277136f / (f4 * f4 * f4);
            float f6 = this.D ? this.bl() * f5 : this.aQ;
            this.a(f2, f3, f6);
            f4 = 0.91f;
            if (this.D) {
                f4 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.91f;
            }
            if (this.h_()) {
                boolean bl2;
                float f7 = 0.15f;
                if (this.v < (double)(-f7)) {
                    this.v = -f7;
                }
                if (this.v > (double)f7) {
                    this.v = f7;
                }
                if (this.x < (double)(-f7)) {
                    this.x = -f7;
                }
                if (this.x > (double)f7) {
                    this.x = f7;
                }
                this.R = 0.0f;
                if (this.w < -0.15) {
                    this.w = -0.15;
                }
                boolean bl3 = bl2 = this.an() && this instanceof yz;
                if (bl2 && this.w < 0.0) {
                    this.w = 0.0;
                }
            }
            this.d(this.v, this.w, this.x);
            if (this.E && this.h_()) {
                this.w = 0.2;
            }
            this.w = !this.o.E || this.o.d((int)this.s, 0, (int)this.u) && this.o.d((int)((int)this.s), (int)((int)this.u)).d ? (this.w -= 0.08) : (this.t > 0.0 ? -0.1 : 0.0);
            this.w *= (double)0.98f;
            this.v *= (double)f4;
            this.x *= (double)f4;
        }
        this.aE = this.aF;
        double d4 = this.s - this.p;
        double d5 = this.u - this.r;
        float f8 = qh.a(d4 * d4 + d5 * d5) * 4.0f;
        if (f8 > 1.0f) {
            f8 = 1.0f;
        }
        this.aF += (f8 - this.aF) * 0.4f;
        this.aG += this.aF;
    }

    protected boolean bk() {
        return false;
    }

    public float bl() {
        if (this.bk()) {
            return this.bp;
        }
        return 0.1f;
    }

    public void i(float f2) {
        this.bp = f2;
    }

    public boolean n(sa sa2) {
        this.l(sa2);
        return false;
    }

    public boolean bm() {
        return false;
    }

    @Override
    public void h() {
        super.h();
        if (!this.o.E) {
            int n2 = this.aZ();
            if (n2 > 0) {
                if (this.av <= 0) {
                    this.av = 20 * (30 - n2);
                }
                --this.av;
                if (this.av <= 0) {
                    this.p(n2 - 1);
                }
            }
            for (int i2 = 0; i2 < 5; ++i2) {
                add add2 = this.g[i2];
                add add3 = this.q(i2);
                if (add.b(add3, add2)) continue;
                ((mt)this.o).r().a(this, new hz(this.y(), i2, add3));
                if (add2 != null) {
                    this.d.a(add2.D());
                }
                if (add3 != null) {
                    this.d.b(add3.D());
                }
                this.g[i2] = add3 == null ? null : add3.m();
            }
            if (this.aa % 20 == 0) {
                this.aW().g();
            }
        }
        this.e();
        double d2 = this.s - this.p;
        double d3 = this.u - this.r;
        float f2 = (float)(d2 * d2 + d3 * d3);
        float f3 = this.aM;
        float f4 = 0.0f;
        this.aV = this.aW;
        float f5 = 0.0f;
        if (f2 > 0.0025000002f) {
            f5 = 1.0f;
            f4 = (float)Math.sqrt(f2) * 3.0f;
            f3 = (float)Math.atan2(d3, d2) * 180.0f / (float)Math.PI - 90.0f;
        }
        if (this.aD > 0.0f) {
            f3 = this.y;
        }
        if (!this.D) {
            f5 = 0.0f;
        }
        this.aW += (f5 - this.aW) * 0.3f;
        this.o.C.a("headTurn");
        f4 = this.f(f3, f4);
        this.o.C.b();
        this.o.C.a("rangeChecks");
        while (this.y - this.A < -180.0f) {
            this.A -= 360.0f;
        }
        while (this.y - this.A >= 180.0f) {
            this.A += 360.0f;
        }
        while (this.aM - this.aN < -180.0f) {
            this.aN -= 360.0f;
        }
        while (this.aM - this.aN >= 180.0f) {
            this.aN += 360.0f;
        }
        while (this.z - this.B < -180.0f) {
            this.B -= 360.0f;
        }
        while (this.z - this.B >= 180.0f) {
            this.B += 360.0f;
        }
        while (this.aO - this.aP < -180.0f) {
            this.aP -= 360.0f;
        }
        while (this.aO - this.aP >= 180.0f) {
            this.aP += 360.0f;
        }
        this.o.C.b();
        this.aX += f4;
    }

    protected float f(float f2, float f3) {
        boolean bl2;
        float f4 = qh.g(f2 - this.aM);
        this.aM += f4 * 0.3f;
        float f5 = qh.g(this.y - this.aM);
        boolean bl3 = bl2 = f5 < -90.0f || f5 >= 90.0f;
        if (f5 < -75.0f) {
            f5 = -75.0f;
        }
        if (f5 >= 75.0f) {
            f5 = 75.0f;
        }
        this.aM = this.y - f5;
        if (f5 * f5 > 2500.0f) {
            this.aM += f5 * 0.2f;
        }
        if (bl2) {
            f3 *= -1.0f;
        }
        return f3;
    }

    public void e() {
        if (this.bq > 0) {
            --this.bq;
        }
        if (this.bg > 0) {
            double d2 = this.s + (this.bh - this.s) / (double)this.bg;
            double d3 = this.t + (this.bi - this.t) / (double)this.bg;
            double d4 = this.u + (this.bj - this.u) / (double)this.bg;
            double d5 = qh.g(this.bk - (double)this.y);
            this.y = (float)((double)this.y + d5 / (double)this.bg);
            this.z = (float)((double)this.z + (this.bl - (double)this.z) / (double)this.bg);
            --this.bg;
            this.b(d2, d3, d4);
            this.b(this.y, this.z);
        } else if (!this.br()) {
            this.v *= 0.98;
            this.w *= 0.98;
            this.x *= 0.98;
        }
        if (Math.abs(this.v) < 0.005) {
            this.v = 0.0;
        }
        if (Math.abs(this.w) < 0.005) {
            this.w = 0.0;
        }
        if (Math.abs(this.x) < 0.005) {
            this.x = 0.0;
        }
        this.o.C.a("ai");
        if (this.bh()) {
            this.bc = false;
            this.bd = 0.0f;
            this.be = 0.0f;
            this.bf = 0.0f;
        } else if (this.br()) {
            if (this.bk()) {
                this.o.C.a("newAi");
                this.bn();
                this.o.C.b();
            } else {
                this.o.C.a("oldAi");
                this.bq();
                this.o.C.b();
                this.aO = this.y;
            }
        }
        this.o.C.b();
        this.o.C.a("jump");
        if (this.bc) {
            if (this.M() || this.P()) {
                this.w += (double)0.04f;
            } else if (this.D && this.bq == 0) {
                this.bj();
                this.bq = 10;
            }
        } else {
            this.bq = 0;
        }
        this.o.C.b();
        this.o.C.a("travel");
        this.bd *= 0.98f;
        this.be *= 0.98f;
        this.bf *= 0.9f;
        this.e(this.bd, this.be);
        this.o.C.b();
        this.o.C.a("push");
        if (!this.o.E) {
            this.bo();
        }
        this.o.C.b();
    }

    protected void bn() {
    }

    protected void bo() {
        List list = this.o.b((sa)this, this.C.b(0.2f, 0.0, 0.2f));
        if (list != null && !list.isEmpty()) {
            for (int i2 = 0; i2 < list.size(); ++i2) {
                sa sa2 = (sa)list.get(i2);
                if (!sa2.S()) continue;
                this.o(sa2);
            }
        }
    }

    protected void o(sa sa2) {
        sa2.g(this);
    }

    @Override
    public void ab() {
        super.ab();
        this.aV = this.aW;
        this.aW = 0.0f;
        this.R = 0.0f;
    }

    protected void bp() {
    }

    protected void bq() {
        ++this.aU;
    }

    public void f(boolean bl2) {
        this.bc = bl2;
    }

    public void a(sa sa2, int n2) {
        if (!sa2.K && !this.o.E) {
            mn mn2 = ((mt)this.o).r();
            if (sa2 instanceof xk) {
                mn2.a(sa2, new ij(sa2.y(), this.y()));
            }
            if (sa2 instanceof zc) {
                mn2.a(sa2, new ij(sa2.y(), this.y()));
            }
            if (sa2 instanceof sq) {
                mn2.a(sa2, new ij(sa2.y(), this.y()));
            }
        }
    }

    public boolean p(sa sa2) {
        return this.o.a(azw.a(this.s, this.t + (double)this.g(), this.u), azw.a(sa2.s, sa2.t + (double)sa2.g(), sa2.u)) == null;
    }

    @Override
    public azw ag() {
        return this.j(1.0f);
    }

    public azw j(float f2) {
        if (f2 == 1.0f) {
            float f3 = qh.b(-this.y * ((float)Math.PI / 180) - (float)Math.PI);
            float f4 = qh.a(-this.y * ((float)Math.PI / 180) - (float)Math.PI);
            float f5 = -qh.b(-this.z * ((float)Math.PI / 180));
            float f6 = qh.a(-this.z * ((float)Math.PI / 180));
            return azw.a(f4 * f5, f6, f3 * f5);
        }
        float f7 = this.B + (this.z - this.B) * f2;
        float f8 = this.A + (this.y - this.A) * f2;
        float f9 = qh.b(-f8 * ((float)Math.PI / 180) - (float)Math.PI);
        float f10 = qh.a(-f8 * ((float)Math.PI / 180) - (float)Math.PI);
        float f11 = -qh.b(-f7 * ((float)Math.PI / 180));
        float f12 = qh.a(-f7 * ((float)Math.PI / 180));
        return azw.a(f10 * f11, f12, f9 * f11);
    }

    public boolean br() {
        return !this.o.E;
    }

    @Override
    public boolean R() {
        return !this.K;
    }

    @Override
    public boolean S() {
        return !this.K;
    }

    @Override
    public float g() {
        return this.N * 0.85f;
    }

    @Override
    protected void Q() {
        this.H = this.Z.nextDouble() >= this.a(yj.c).e();
    }

    @Override
    public float au() {
        return this.aO;
    }

    public float bs() {
        return this.br;
    }

    public void m(float f2) {
        if (f2 < 0.0f) {
            f2 = 0.0f;
        }
        this.br = f2;
    }

    public bae bt() {
        return null;
    }

    public boolean c(sv sv2) {
        return this.a(sv2.bt());
    }

    public boolean a(bae bae2) {
        if (this.bt() != null) {
            return this.bt().a(bae2);
        }
        return false;
    }

    public void bu() {
    }

    public void bv() {
    }
}

