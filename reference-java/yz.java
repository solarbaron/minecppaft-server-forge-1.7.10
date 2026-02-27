/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;
import com.mojang.authlib.GameProfile;
import java.util.Collection;
import java.util.List;
import java.util.UUID;

public abstract class yz
extends sv
implements ac {
    public yx bm = new yx(this);
    private aav a = new aav();
    public zs bn;
    public zs bo;
    protected zr bp = new zr();
    protected int bq;
    public float br;
    public float bs;
    public int bt;
    public double bu;
    public double bv;
    public double bw;
    public double bx;
    public double by;
    public double bz;
    protected boolean bA;
    public r bB;
    private int b;
    public float bC;
    public float bD;
    private r c;
    private boolean d;
    private r e;
    public yw bE = new yw();
    public int bF;
    public int bG;
    public float bH;
    private add f;
    private int g;
    protected float bI = 0.1f;
    protected float bJ = 0.02f;
    private int h;
    private final GameProfile i;
    public xe bK;

    public yz(ahb ahb2, GameProfile gameProfile) {
        super(ahb2);
        this.ar = yz.a(gameProfile);
        this.i = gameProfile;
        this.bo = this.bn = new aap(this.bm, !ahb2.E, this);
        this.L = 1.62f;
        r r2 = ahb2.K();
        this.b((double)r2.a + 0.5, r2.b + 1, (double)r2.c + 0.5, 0.0f, 0.0f);
        this.aZ = 180.0f;
        this.ab = 20;
    }

    @Override
    protected void aD() {
        super.aD();
        this.bc().b(yj.e).a(1.0);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, (Object)0);
        this.af.a(17, Float.valueOf(0.0f));
        this.af.a(18, (Object)0);
    }

    public boolean by() {
        return this.f != null;
    }

    public void bA() {
        if (this.f != null) {
            this.f.b(this.o, this, this.g);
        }
        this.bB();
    }

    public void bB() {
        this.f = null;
        this.g = 0;
        if (!this.o.E) {
            this.e(false);
        }
    }

    public boolean bC() {
        return this.by() && this.f.b().d(this.f) == aei.d;
    }

    @Override
    public void h() {
        if (this.f != null) {
            add add2 = this.bm.h();
            if (add2 == this.f) {
                if (this.g <= 25 && this.g % 4 == 0) {
                    this.c(add2, 5);
                }
                if (--this.g == 0 && !this.o.E) {
                    this.p();
                }
            } else {
                this.bB();
            }
        }
        if (this.bt > 0) {
            --this.bt;
        }
        if (this.bm()) {
            ++this.b;
            if (this.b > 100) {
                this.b = 100;
            }
            if (!this.o.E) {
                if (!this.j()) {
                    this.a(true, true, false);
                } else if (this.o.w()) {
                    this.a(false, true, true);
                }
            }
        } else if (this.b > 0) {
            ++this.b;
            if (this.b >= 110) {
                this.b = 0;
            }
        }
        super.h();
        if (!this.o.E && this.bo != null && !this.bo.a(this)) {
            this.k();
            this.bo = this.bn;
        }
        if (this.al() && this.bE.a) {
            this.F();
        }
        this.bu = this.bx;
        this.bv = this.by;
        this.bw = this.bz;
        double d2 = this.s - this.bx;
        double d3 = this.t - this.by;
        double d4 = this.u - this.bz;
        double d5 = 10.0;
        if (d2 > d5) {
            this.bu = this.bx = this.s;
        }
        if (d4 > d5) {
            this.bw = this.bz = this.u;
        }
        if (d3 > d5) {
            this.bv = this.by = this.t;
        }
        if (d2 < -d5) {
            this.bu = this.bx = this.s;
        }
        if (d4 < -d5) {
            this.bw = this.bz = this.u;
        }
        if (d3 < -d5) {
            this.bv = this.by = this.t;
        }
        this.bx += d2 * 0.25;
        this.bz += d4 * 0.25;
        this.by += d3 * 0.25;
        if (this.m == null) {
            this.e = null;
        }
        if (!this.o.E) {
            this.bp.a(this);
            this.a(pp.g, 1);
        }
    }

    @Override
    public int D() {
        return this.bE.a ? 0 : 80;
    }

    @Override
    protected String H() {
        return "game.player.swim";
    }

    @Override
    protected String O() {
        return "game.player.swim.splash";
    }

    @Override
    public int ai() {
        return 10;
    }

    @Override
    public void a(String string, float f2, float f3) {
        this.o.a(this, string, f2, f3);
    }

    protected void c(add add2, int n2) {
        if (add2.o() == aei.c) {
            this.a("random.drink", 0.5f, this.o.s.nextFloat() * 0.1f + 0.9f);
        }
        if (add2.o() == aei.b) {
            for (int i2 = 0; i2 < n2; ++i2) {
                azw azw2 = azw.a(((double)this.Z.nextFloat() - 0.5) * 0.1, Math.random() * 0.1 + 0.1, 0.0);
                azw2.a(-this.z * (float)Math.PI / 180.0f);
                azw2.b(-this.y * (float)Math.PI / 180.0f);
                azw azw3 = azw.a(((double)this.Z.nextFloat() - 0.5) * 0.3, (double)(-this.Z.nextFloat()) * 0.6 - 0.3, 0.6);
                azw3.a(-this.z * (float)Math.PI / 180.0f);
                azw3.b(-this.y * (float)Math.PI / 180.0f);
                azw3 = azw3.c(this.s, this.t + (double)this.g(), this.u);
                String string = "iconcrack_" + adb.b(add2.b());
                if (add2.h()) {
                    string = string + "_" + add2.k();
                }
                this.o.a(string, azw3.a, azw3.b, azw3.c, azw2.a, azw2.b + 0.05, azw2.c);
            }
            this.a("random.eat", 0.5f + 0.5f * (float)this.Z.nextInt(2), (this.Z.nextFloat() - this.Z.nextFloat()) * 0.2f + 1.0f);
        }
    }

    protected void p() {
        if (this.f != null) {
            this.c(this.f, 16);
            int n2 = this.f.b;
            add add2 = this.f.b(this.o, this);
            if (add2 != this.f || add2 != null && add2.b != n2) {
                this.bm.a[this.bm.c] = add2;
                if (add2.b == 0) {
                    this.bm.a[this.bm.c] = null;
                }
            }
            this.bB();
        }
    }

    @Override
    protected boolean bh() {
        return this.aS() <= 0.0f || this.bm();
    }

    protected void k() {
        this.bo = this.bn;
    }

    @Override
    public void a(sa sa2) {
        if (this.m != null && sa2 == null) {
            if (!this.o.E) {
                this.m(this.m);
            }
            if (this.m != null) {
                this.m.l = null;
            }
            this.m = null;
            return;
        }
        super.a(sa2);
    }

    @Override
    public void ab() {
        if (!this.o.E && this.an()) {
            this.a((sa)null);
            this.b(false);
            return;
        }
        double d2 = this.s;
        double d3 = this.t;
        double d4 = this.u;
        float f2 = this.y;
        float f3 = this.z;
        super.ab();
        this.br = this.bs;
        this.bs = 0.0f;
        this.l(this.s - d2, this.t - d3, this.u - d4);
        if (this.m instanceof wo) {
            this.z = f3;
            this.y = f2;
            this.aM = ((wo)this.m).aM;
        }
    }

    @Override
    protected void bq() {
        super.bq();
        this.bb();
    }

    @Override
    public void e() {
        if (this.bq > 0) {
            --this.bq;
        }
        if (this.o.r == rd.a && this.aS() < this.aY() && this.o.O().b("naturalRegeneration") && this.aa % 20 * 12 == 0) {
            this.f(1.0f);
        }
        this.bm.k();
        this.br = this.bs;
        super.e();
        ti ti2 = this.a(yj.d);
        if (!this.o.E) {
            ti2.a(this.bE.b());
        }
        this.aQ = this.bJ;
        if (this.ao()) {
            this.aQ = (float)((double)this.aQ + (double)this.bJ * 0.3);
        }
        this.i((float)ti2.e());
        float f2 = qh.a(this.v * this.v + this.x * this.x);
        float f3 = (float)Math.atan(-this.w * (double)0.2f) * 15.0f;
        if (f2 > 0.1f) {
            f2 = 0.1f;
        }
        if (!this.D || this.aS() <= 0.0f) {
            f2 = 0.0f;
        }
        if (this.D || this.aS() <= 0.0f) {
            f3 = 0.0f;
        }
        this.bs += (f2 - this.bs) * 0.4f;
        this.aJ += (f3 - this.aJ) * 0.8f;
        if (this.aS() > 0.0f) {
            azt azt2 = null;
            azt2 = this.m != null && !this.m.K ? this.C.a(this.m.C).b(1.0, 0.0, 1.0) : this.C.b(1.0, 0.5, 1.0);
            List list = this.o.b((sa)this, azt2);
            if (list != null) {
                for (int i2 = 0; i2 < list.size(); ++i2) {
                    sa sa2 = (sa)list.get(i2);
                    if (sa2.K) continue;
                    this.d(sa2);
                }
            }
        }
    }

    private void d(sa sa2) {
        sa2.b_(this);
    }

    public int bD() {
        return this.af.c(18);
    }

    public void c(int n2) {
        this.af.b(18, n2);
    }

    public void s(int n2) {
        int n3 = this.bD();
        this.af.b(18, n3 + n2);
    }

    @Override
    public void a(ro ro2) {
        super.a(ro2);
        this.a(0.2f, 0.2f);
        this.b(this.s, this.t, this.u);
        this.w = 0.1f;
        if (this.b_().equals("Notch")) {
            this.a(new add(ade.e, 1), true, false);
        }
        if (!this.o.O().b("keepInventory")) {
            this.bm.m();
        }
        if (ro2 != null) {
            this.v = -qh.b((this.az + this.y) * (float)Math.PI / 180.0f) * 0.1f;
            this.x = -qh.a((this.az + this.y) * (float)Math.PI / 180.0f) * 0.1f;
        } else {
            this.x = 0.0;
            this.v = 0.0;
        }
        this.L = 0.1f;
        this.a(pp.v, 1);
    }

    @Override
    protected String aT() {
        return "game.player.hurt";
    }

    @Override
    protected String aU() {
        return "game.player.die";
    }

    @Override
    public void b(sa sa2, int n2) {
        this.s(n2);
        Collection collection = this.bU().a(bah.e);
        if (sa2 instanceof yz) {
            this.a(pp.y, 1);
            collection.addAll(this.bU().a(bah.d));
        } else {
            this.a(pp.w, 1);
        }
        for (azx azx2 : collection) {
            azz azz2 = this.bU().a(this.b_(), azx2);
            azz2.a();
        }
    }

    public xk a(boolean bl2) {
        return this.a(this.bm.a(this.bm.c, bl2 && this.bm.h() != null ? this.bm.h().b : 1), false, true);
    }

    public xk a(add add2, boolean bl2) {
        return this.a(add2, false, false);
    }

    public xk a(add add2, boolean bl2, boolean bl3) {
        if (add2 == null) {
            return null;
        }
        if (add2.b == 0) {
            return null;
        }
        xk xk2 = new xk(this.o, this.s, this.t - (double)0.3f + (double)this.g(), this.u, add2);
        xk2.b = 40;
        if (bl3) {
            xk2.b(this.b_());
        }
        float f2 = 0.1f;
        if (bl2) {
            float f3 = this.Z.nextFloat() * 0.5f;
            float f4 = this.Z.nextFloat() * (float)Math.PI * 2.0f;
            xk2.v = -qh.a(f4) * f3;
            xk2.x = qh.b(f4) * f3;
            xk2.w = 0.2f;
        } else {
            f2 = 0.3f;
            xk2.v = -qh.a(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI) * f2;
            xk2.x = qh.b(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI) * f2;
            xk2.w = -qh.a(this.z / 180.0f * (float)Math.PI) * f2 + 0.1f;
            f2 = 0.02f;
            float f5 = this.Z.nextFloat() * (float)Math.PI * 2.0f;
            xk2.v += Math.cos(f5) * (double)(f2 *= this.Z.nextFloat());
            xk2.w += (double)((this.Z.nextFloat() - this.Z.nextFloat()) * 0.1f);
            xk2.x += Math.sin(f5) * (double)f2;
        }
        this.a(xk2);
        this.a(pp.s, 1);
        return xk2;
    }

    protected void a(xk xk2) {
        this.o.d(xk2);
    }

    public float a(aji aji2, boolean bl2) {
        float f2 = this.bm.a(aji2);
        if (f2 > 1.0f) {
            int n2 = afv.c(this);
            add add2 = this.bm.h();
            if (n2 > 0 && add2 != null) {
                float f3 = n2 * n2 + 1;
                f2 = add2.b(aji2) || f2 > 1.0f ? (f2 += f3) : (f2 += f3 * 0.08f);
            }
        }
        if (this.a(rv.e)) {
            f2 *= 1.0f + (float)(this.b(rv.e).c() + 1) * 0.2f;
        }
        if (this.a(rv.f)) {
            f2 *= 1.0f - (float)(this.b(rv.f).c() + 1) * 0.2f;
        }
        if (this.a(awt.h) && !afv.j(this)) {
            f2 /= 5.0f;
        }
        if (!this.D) {
            f2 /= 5.0f;
        }
        return f2;
    }

    public boolean a(aji aji2) {
        return this.bm.b(aji2);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.ar = yz.a(this.i);
        dq dq2 = dh2.c("Inventory", 10);
        this.bm.b(dq2);
        this.bm.c = dh2.f("SelectedItemSlot");
        this.bA = dh2.n("Sleeping");
        this.b = dh2.e("SleepTimer");
        this.bH = dh2.h("XpP");
        this.bF = dh2.f("XpLevel");
        this.bG = dh2.f("XpTotal");
        this.c(dh2.f("Score"));
        if (this.bA) {
            this.bB = new r(qh.c(this.s), qh.c(this.t), qh.c(this.u));
            this.a(true, true, false);
        }
        if (dh2.b("SpawnX", 99) && dh2.b("SpawnY", 99) && dh2.b("SpawnZ", 99)) {
            this.c = new r(dh2.f("SpawnX"), dh2.f("SpawnY"), dh2.f("SpawnZ"));
            this.d = dh2.n("SpawnForced");
        }
        this.bp.a(dh2);
        this.bE.b(dh2);
        if (dh2.b("EnderItems", 9)) {
            dq dq3 = dh2.c("EnderItems", 10);
            this.a.a(dq3);
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Inventory", this.bm.a(new dq()));
        dh2.a("SelectedItemSlot", this.bm.c);
        dh2.a("Sleeping", this.bA);
        dh2.a("SleepTimer", (short)this.b);
        dh2.a("XpP", this.bH);
        dh2.a("XpLevel", this.bF);
        dh2.a("XpTotal", this.bG);
        dh2.a("Score", this.bD());
        if (this.c != null) {
            dh2.a("SpawnX", this.c.a);
            dh2.a("SpawnY", this.c.b);
            dh2.a("SpawnZ", this.c.c);
            dh2.a("SpawnForced", this.d);
        }
        this.bp.b(dh2);
        this.bE.a(dh2);
        dh2.a("EnderItems", this.a.h());
    }

    public void a(rb rb2) {
    }

    public void a(api api2) {
    }

    public void a(xr xr2) {
    }

    public void a(wi wi2, rb rb2) {
    }

    public void a(int n2, int n3, int n4, String string) {
    }

    public void c(int n2, int n3, int n4) {
    }

    public void b(int n2, int n3, int n4) {
    }

    @Override
    public float g() {
        return 0.12f;
    }

    protected void e_() {
        this.L = 1.62f;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        if (this.bE.a && !ro2.g()) {
            return false;
        }
        this.aU = 0;
        if (this.aS() <= 0.0f) {
            return false;
        }
        if (this.bm() && !this.o.E) {
            this.a(true, true, false);
        }
        if (ro2.r()) {
            if (this.o.r == rd.a) {
                f2 = 0.0f;
            }
            if (this.o.r == rd.b) {
                f2 = f2 / 2.0f + 1.0f;
            }
            if (this.o.r == rd.d) {
                f2 = f2 * 3.0f / 2.0f;
            }
        }
        if (f2 == 0.0f) {
            return false;
        }
        sa sa2 = ro2.j();
        if (sa2 instanceof zc && ((zc)sa2).c != null) {
            sa2 = ((zc)sa2).c;
        }
        this.a(pp.u, Math.round(f2 * 10.0f));
        return super.a(ro2, f2);
    }

    public boolean a(yz yz2) {
        bae bae2 = this.bt();
        bae bae3 = yz2.bt();
        if (bae2 == null) {
            return true;
        }
        if (!bae2.a(bae3)) {
            return true;
        }
        return bae2.g();
    }

    @Override
    protected void h(float f2) {
        this.bm.a(f2);
    }

    @Override
    public int aV() {
        return this.bm.l();
    }

    public float bE() {
        int n2 = 0;
        for (add add2 : this.bm.b) {
            if (add2 == null) continue;
            ++n2;
        }
        return (float)n2 / (float)this.bm.b.length;
    }

    @Override
    protected void d(ro ro2, float f2) {
        if (this.aw()) {
            return;
        }
        if (!ro2.e() && this.bC() && f2 > 0.0f) {
            f2 = (1.0f + f2) * 0.5f;
        }
        f2 = this.b(ro2, f2);
        float f3 = f2 = this.c(ro2, f2);
        f2 = Math.max(f2 - this.bs(), 0.0f);
        this.m(this.bs() - (f3 - f2));
        if (f2 == 0.0f) {
            return;
        }
        this.a(ro2.f());
        float f4 = this.aS();
        this.g(this.aS() - f2);
        this.aW().a(ro2, f4, f2);
    }

    public void a(apg apg2) {
    }

    public void a(apb apb2) {
    }

    public void a(aor aor2) {
    }

    public void a(agp agp2) {
    }

    public void a(aov aov2) {
    }

    public void a(aoq aoq2) {
    }

    public void a(agm agm2, String string) {
    }

    public void b(add add2) {
    }

    public boolean q(sa sa2) {
        add add2;
        add add3 = this.bF();
        add add4 = add2 = add3 != null ? add3.m() : null;
        if (sa2.c(this)) {
            if (add3 != null && add3 == this.bF()) {
                if (add3.b <= 0 && !this.bE.d) {
                    this.bG();
                } else if (add3.b < add2.b && this.bE.d) {
                    add3.b = add2.b;
                }
            }
            return true;
        }
        if (add3 != null && sa2 instanceof sv) {
            if (this.bE.d) {
                add3 = add2;
            }
            if (add3.a(this, (sv)sa2)) {
                if (add3.b <= 0 && !this.bE.d) {
                    this.bG();
                }
                return true;
            }
        }
        return false;
    }

    public add bF() {
        return this.bm.h();
    }

    public void bG() {
        this.bm.a(this.bm.c, null);
    }

    @Override
    public double ad() {
        return this.L - 0.5f;
    }

    public void r(sa sa2) {
        if (!sa2.av()) {
            return;
        }
        if (sa2.j(this)) {
            return;
        }
        float f2 = (float)this.a(yj.e).e();
        int n2 = 0;
        float f3 = 0.0f;
        if (sa2 instanceof sv) {
            f3 = afv.a((sv)this, (sv)sa2);
            n2 += afv.b((sv)this, (sv)sa2);
        }
        if (this.ao()) {
            ++n2;
        }
        if (f2 > 0.0f || f3 > 0.0f) {
            boolean bl2;
            boolean bl3;
            boolean bl4 = bl3 = this.R > 0.0f && !this.D && !this.h_() && !this.M() && !this.a(rv.q) && this.m == null && sa2 instanceof sv;
            if (bl3 && f2 > 0.0f) {
                f2 *= 1.5f;
            }
            f2 += f3;
            boolean bl5 = false;
            int n3 = afv.a(this);
            if (sa2 instanceof sv && n3 > 0 && !sa2.al()) {
                bl5 = true;
                sa2.e(1);
            }
            if (bl2 = sa2.a(ro.a(this), f2)) {
                wx wx2;
                if (n2 > 0) {
                    sa2.g(-qh.a(this.y * (float)Math.PI / 180.0f) * (float)n2 * 0.5f, 0.1, qh.b(this.y * (float)Math.PI / 180.0f) * (float)n2 * 0.5f);
                    this.v *= 0.6;
                    this.x *= 0.6;
                    this.c(false);
                }
                if (bl3) {
                    this.b(sa2);
                }
                if (f3 > 0.0f) {
                    this.c(sa2);
                }
                if (f2 >= 18.0f) {
                    this.a(pc.F);
                }
                this.l(sa2);
                if (sa2 instanceof sv) {
                    afv.a((sv)sa2, (sa)this);
                }
                afv.b((sv)this, sa2);
                add add2 = this.bF();
                sa sa3 = sa2;
                if (sa2 instanceof wy && (wx2 = ((wy)sa2).a) != null && wx2 instanceof sv) {
                    sa3 = (sv)((Object)wx2);
                }
                if (add2 != null && sa3 instanceof sv) {
                    add2.a((sv)sa3, this);
                    if (add2.b <= 0) {
                        this.bG();
                    }
                }
                if (sa2 instanceof sv) {
                    this.a(pp.t, Math.round(f2 * 10.0f));
                    if (n3 > 0) {
                        sa2.e(n3 * 4);
                    }
                }
                this.a(0.3f);
            } else if (bl5) {
                sa2.F();
            }
        }
    }

    public void b(sa sa2) {
    }

    public void c(sa sa2) {
    }

    @Override
    public void B() {
        super.B();
        this.bn.b(this);
        if (this.bo != null) {
            this.bo.b(this);
        }
    }

    @Override
    public boolean aa() {
        return !this.bA && super.aa();
    }

    public GameProfile bJ() {
        return this.i;
    }

    public za a(int n2, int n3, int n4) {
        if (!this.o.E) {
            if (this.bm() || !this.Z()) {
                return za.e;
            }
            if (!this.o.t.d()) {
                return za.b;
            }
            if (this.o.w()) {
                return za.c;
            }
            if (Math.abs(this.s - (double)n2) > 3.0 || Math.abs(this.t - (double)n3) > 2.0 || Math.abs(this.u - (double)n4) > 3.0) {
                return za.d;
            }
            double d2 = 8.0;
            double d3 = 5.0;
            List list = this.o.a(yg.class, azt.a((double)n2 - d2, (double)n3 - d3, (double)n4 - d2, (double)n2 + d2, (double)n3 + d3, (double)n4 + d2));
            if (!list.isEmpty()) {
                return za.f;
            }
        }
        if (this.am()) {
            this.a((sa)null);
        }
        this.a(0.2f, 0.2f);
        this.L = 0.2f;
        if (this.o.d(n2, n3, n4)) {
            int n5 = this.o.e(n2, n3, n4);
            int n6 = ajh.l(n5);
            float f2 = 0.5f;
            float f3 = 0.5f;
            switch (n6) {
                case 0: {
                    f3 = 0.9f;
                    break;
                }
                case 2: {
                    f3 = 0.1f;
                    break;
                }
                case 1: {
                    f2 = 0.1f;
                    break;
                }
                case 3: {
                    f2 = 0.9f;
                }
            }
            this.w(n6);
            this.b((float)n2 + f2, (float)n3 + 0.9375f, (float)n4 + f3);
        } else {
            this.b((float)n2 + 0.5f, (float)n3 + 0.9375f, (float)n4 + 0.5f);
        }
        this.bA = true;
        this.b = 0;
        this.bB = new r(n2, n3, n4);
        this.w = 0.0;
        this.x = 0.0;
        this.v = 0.0;
        if (!this.o.E) {
            this.o.c();
        }
        return za.a;
    }

    private void w(int n2) {
        this.bC = 0.0f;
        this.bD = 0.0f;
        switch (n2) {
            case 0: {
                this.bD = -1.8f;
                break;
            }
            case 2: {
                this.bD = 1.8f;
                break;
            }
            case 1: {
                this.bC = 1.8f;
                break;
            }
            case 3: {
                this.bC = -1.8f;
            }
        }
    }

    public void a(boolean bl2, boolean bl3, boolean bl4) {
        this.a(0.6f, 1.8f);
        this.e_();
        r r2 = this.bB;
        r r3 = this.bB;
        if (r2 != null && this.o.a(r2.a, r2.b, r2.c) == ajn.C) {
            ajh.a(this.o, r2.a, r2.b, r2.c, false);
            r3 = ajh.a(this.o, r2.a, r2.b, r2.c, 0);
            if (r3 == null) {
                r3 = new r(r2.a, r2.b + 1, r2.c);
            }
            this.b((float)r3.a + 0.5f, (float)r3.b + this.L + 0.1f, (float)r3.c + 0.5f);
        }
        this.bA = false;
        if (!this.o.E && bl3) {
            this.o.c();
        }
        this.b = bl2 ? 0 : 100;
        if (bl4) {
            this.a(this.bB, false);
        }
    }

    private boolean j() {
        return this.o.a(this.bB.a, this.bB.b, this.bB.c) == ajn.C;
    }

    public static r a(ahb ahb2, r r2, boolean bl2) {
        apu apu2 = ahb2.L();
        apu2.c(r2.a - 3 >> 4, r2.c - 3 >> 4);
        apu2.c(r2.a + 3 >> 4, r2.c - 3 >> 4);
        apu2.c(r2.a - 3 >> 4, r2.c + 3 >> 4);
        apu2.c(r2.a + 3 >> 4, r2.c + 3 >> 4);
        if (ahb2.a(r2.a, r2.b, r2.c) != ajn.C) {
            boolean bl3;
            awt awt2 = ahb2.a(r2.a, r2.b, r2.c).o();
            awt awt3 = ahb2.a(r2.a, r2.b + 1, r2.c).o();
            boolean bl4 = !awt2.a() && !awt2.d();
            boolean bl5 = bl3 = !awt3.a() && !awt3.d();
            if (bl2 && bl4 && bl3) {
                return r2;
            }
            return null;
        }
        r r3 = ajh.a(ahb2, r2.a, r2.b, r2.c, 0);
        return r3;
    }

    @Override
    public boolean bm() {
        return this.bA;
    }

    public boolean bL() {
        return this.bA && this.b >= 100;
    }

    protected void b(int n2, boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 1 << n2));
        } else {
            this.af.b(16, (byte)(by2 & ~(1 << n2)));
        }
    }

    public void b(fj fj2) {
    }

    public r bN() {
        return this.c;
    }

    public boolean bO() {
        return this.d;
    }

    public void a(r r2, boolean bl2) {
        if (r2 != null) {
            this.c = new r(r2);
            this.d = bl2;
        } else {
            this.c = null;
            this.d = false;
        }
    }

    public void a(ph ph2) {
        this.a(ph2, 1);
    }

    public void a(ph ph2, int n2) {
    }

    @Override
    public void bj() {
        super.bj();
        this.a(pp.r, 1);
        if (this.ao()) {
            this.a(0.8f);
        } else {
            this.a(0.2f);
        }
    }

    @Override
    public void e(float f2, float f3) {
        double d2 = this.s;
        double d3 = this.t;
        double d4 = this.u;
        if (this.bE.b && this.m == null) {
            double d5 = this.w;
            float f4 = this.aQ;
            this.aQ = this.bE.a();
            super.e(f2, f3);
            this.w = d5 * 0.6;
            this.aQ = f4;
        } else {
            super.e(f2, f3);
        }
        this.k(this.s - d2, this.t - d3, this.u - d4);
    }

    @Override
    public float bl() {
        return (float)this.a(yj.d).e();
    }

    public void k(double d2, double d3, double d4) {
        if (this.m != null) {
            return;
        }
        if (this.a(awt.h)) {
            int n2 = Math.round(qh.a(d2 * d2 + d3 * d3 + d4 * d4) * 100.0f);
            if (n2 > 0) {
                this.a(pp.m, n2);
                this.a(0.015f * (float)n2 * 0.01f);
            }
        } else if (this.M()) {
            int n3 = Math.round(qh.a(d2 * d2 + d4 * d4) * 100.0f);
            if (n3 > 0) {
                this.a(pp.i, n3);
                this.a(0.015f * (float)n3 * 0.01f);
            }
        } else if (this.h_()) {
            if (d3 > 0.0) {
                this.a(pp.k, (int)Math.round(d3 * 100.0));
            }
        } else if (this.D) {
            int n4 = Math.round(qh.a(d2 * d2 + d4 * d4) * 100.0f);
            if (n4 > 0) {
                this.a(pp.h, n4);
                if (this.ao()) {
                    this.a(0.099999994f * (float)n4 * 0.01f);
                } else {
                    this.a(0.01f * (float)n4 * 0.01f);
                }
            }
        } else {
            int n5 = Math.round(qh.a(d2 * d2 + d4 * d4) * 100.0f);
            if (n5 > 25) {
                this.a(pp.l, n5);
            }
        }
    }

    private void l(double d2, double d3, double d4) {
        int n2;
        if (this.m != null && (n2 = Math.round(qh.a(d2 * d2 + d3 * d3 + d4 * d4) * 100.0f)) > 0) {
            if (this.m instanceof xl) {
                this.a(pp.n, n2);
                if (this.e == null) {
                    this.e = new r(qh.c(this.s), qh.c(this.t), qh.c(this.u));
                } else if ((double)this.e.e(qh.c(this.s), qh.c(this.t), qh.c(this.u)) >= 1000000.0) {
                    this.a(pc.q, 1);
                }
            } else if (this.m instanceof xi) {
                this.a(pp.o, n2);
            } else if (this.m instanceof wo) {
                this.a(pp.p, n2);
            } else if (this.m instanceof wi) {
                this.a(pp.q, n2);
            }
        }
    }

    @Override
    protected void b(float f2) {
        if (this.bE.c) {
            return;
        }
        if (f2 >= 2.0f) {
            this.a(pp.j, (int)Math.round((double)f2 * 100.0));
        }
        super.b(f2);
    }

    @Override
    protected String o(int n2) {
        if (n2 > 4) {
            return "game.player.hurt.fall.big";
        }
        return "game.player.hurt.fall.small";
    }

    @Override
    public void a(sv sv2) {
        int n2;
        sh sh2;
        if (sv2 instanceof yb) {
            this.a(pc.s);
        }
        if ((sh2 = (sh)sg.a.get(n2 = sg.a(sv2))) != null) {
            this.a(sh2.d, 1);
        }
    }

    @Override
    public void as() {
        if (!this.bE.b) {
            super.as();
        }
    }

    public add r(int n2) {
        return this.bm.d(n2);
    }

    public void v(int n2) {
        this.s(n2);
        int n3 = Integer.MAX_VALUE - this.bG;
        if (n2 > n3) {
            n2 = n3;
        }
        this.bH += (float)n2 / (float)this.bP();
        this.bG += n2;
        while (this.bH >= 1.0f) {
            this.bH = (this.bH - 1.0f) * (float)this.bP();
            this.a(1);
            this.bH /= (float)this.bP();
        }
    }

    public void a(int n2) {
        this.bF += n2;
        if (this.bF < 0) {
            this.bF = 0;
            this.bH = 0.0f;
            this.bG = 0;
        }
        if (n2 > 0 && this.bF % 5 == 0 && (float)this.h < (float)this.aa - 100.0f) {
            float f2 = this.bF > 30 ? 1.0f : (float)this.bF / 30.0f;
            this.o.a((sa)this, "random.levelup", f2 * 0.75f, 1.0f);
            this.h = this.aa;
        }
    }

    public int bP() {
        if (this.bF >= 30) {
            return 62 + (this.bF - 30) * 7;
        }
        if (this.bF >= 15) {
            return 17 + (this.bF - 15) * 3;
        }
        return 17;
    }

    public void a(float f2) {
        if (this.bE.a) {
            return;
        }
        if (!this.o.E) {
            this.bp.a(f2);
        }
    }

    public zr bQ() {
        return this.bp;
    }

    public boolean g(boolean bl2) {
        return (bl2 || this.bp.c()) && !this.bE.a;
    }

    public boolean bR() {
        return this.aS() > 0.0f && this.aS() < this.aY();
    }

    public void a(add add2, int n2) {
        if (add2 == this.f) {
            return;
        }
        this.f = add2;
        this.g = n2;
        if (!this.o.E) {
            this.e(true);
        }
    }

    public boolean d(int n2, int n3, int n4) {
        if (this.bE.e) {
            return true;
        }
        aji aji2 = this.o.a(n2, n3, n4);
        if (aji2.o() != awt.a) {
            add add2;
            if (aji2.o().q()) {
                return true;
            }
            if (this.bF() != null && ((add2 = this.bF()).b(aji2) || add2.a(aji2) > 1.0f)) {
                return true;
            }
        }
        return false;
    }

    public boolean a(int n2, int n3, int n4, int n5, add add2) {
        if (this.bE.e) {
            return true;
        }
        if (add2 != null) {
            return add2.z();
        }
        return false;
    }

    @Override
    protected int e(yz yz2) {
        if (this.o.O().b("keepInventory")) {
            return 0;
        }
        int n2 = this.bF * 7;
        if (n2 > 100) {
            return 100;
        }
        return n2;
    }

    @Override
    protected boolean aH() {
        return true;
    }

    public void a(yz yz2, boolean bl2) {
        if (bl2) {
            this.bm.b(yz2.bm);
            this.g(yz2.aS());
            this.bp = yz2.bp;
            this.bF = yz2.bF;
            this.bG = yz2.bG;
            this.bH = yz2.bH;
            this.c(yz2.bD());
            this.aq = yz2.aq;
        } else if (this.o.O().b("keepInventory")) {
            this.bm.b(yz2.bm);
            this.bF = yz2.bF;
            this.bG = yz2.bG;
            this.bH = yz2.bH;
            this.c(yz2.bD());
        }
        this.a = yz2.a;
    }

    @Override
    protected boolean g_() {
        return !this.bE.b;
    }

    public void q() {
    }

    public void a(ahk ahk2) {
    }

    @Override
    public String b_() {
        return this.i.getName();
    }

    @Override
    public ahb d() {
        return this.o;
    }

    public aav bS() {
        return this.a;
    }

    @Override
    public add q(int n2) {
        if (n2 == 0) {
            return this.bm.h();
        }
        return this.bm.b[n2 - 1];
    }

    @Override
    public add be() {
        return this.bm.h();
    }

    @Override
    public void c(int n2, add add2) {
        this.bm.b[n2] = add2;
    }

    @Override
    public add[] ak() {
        return this.bm.b;
    }

    @Override
    public boolean aC() {
        return !this.bE.b;
    }

    public bac bU() {
        return this.o.W();
    }

    @Override
    public bae bt() {
        return this.bU().i(this.b_());
    }

    @Override
    public fj c_() {
        fq fq2 = new fq(azy.a(this.bt(), this.b_()));
        fq2.b().a(new fh(fi.e, "/msg " + this.b_() + " "));
        return fq2;
    }

    @Override
    public void m(float f2) {
        if (f2 < 0.0f) {
            f2 = 0.0f;
        }
        this.z().b(17, Float.valueOf(f2));
    }

    @Override
    public float bs() {
        return this.z().d(17);
    }

    public static UUID a(GameProfile gameProfile) {
        UUID uUID = gameProfile.getId();
        if (uUID == null) {
            uUID = UUID.nameUUIDFromBytes(("OfflinePlayer:" + gameProfile.getName()).getBytes(Charsets.UTF_8));
        }
        return uUID;
    }
}

