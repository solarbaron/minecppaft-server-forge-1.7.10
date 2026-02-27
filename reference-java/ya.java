/*
 * Decompiled with CFR 0.152.
 */
import java.util.UUID;

public class ya
extends yg {
    private static final UUID bp = UUID.fromString("020E0DFB-87AE-4653-9556-831010E291A0");
    private static final tj bq = new tj(bp, "Attacking speed boost", 6.2f, 0).a(false);
    private static boolean[] br = new boolean[256];
    private int bs;
    private int bt;
    private sa bu;
    private boolean bv;

    public ya(ahb ahb2) {
        super(ahb2);
        this.a(0.6f, 2.9f);
        this.W = 1.0f;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(40.0);
        this.a(yj.d).a(0.3f);
        this.a(yj.e).a(7.0);
    }

    @Override
    protected void c() {
        super.c();
        this.af.a(16, new Byte(0));
        this.af.a(17, new Byte(0));
        this.af.a(18, new Byte(0));
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("carried", (short)aji.b(this.cb()));
        dh2.a("carriedData", (short)this.cc());
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.a(aji.e(dh2.e("carried")));
        this.a(dh2.e("carriedData"));
    }

    @Override
    protected sa bR() {
        yz yz2 = this.o.b((sa)this, 64.0);
        if (yz2 != null) {
            if (this.f(yz2)) {
                this.bv = true;
                if (this.bt == 0) {
                    this.o.a(yz2.s, yz2.t, yz2.u, "mob.endermen.stare", 1.0f, 1.0f);
                }
                if (this.bt++ == 5) {
                    this.bt = 0;
                    this.a(true);
                    return yz2;
                }
            } else {
                this.bt = 0;
            }
        }
        return null;
    }

    private boolean f(yz yz2) {
        add add2 = yz2.bm.b[3];
        if (add2 != null && add2.b() == adb.a(ajn.aK)) {
            return false;
        }
        azw azw2 = yz2.j(1.0f).a();
        azw azw3 = azw.a(this.s - yz2.s, this.C.b + (double)(this.N / 2.0f) - (yz2.t + (double)yz2.g()), this.u - yz2.u);
        double d2 = azw3.b();
        double d3 = azw2.b(azw3 = azw3.a());
        return d3 > 1.0 - 0.025 / d2 && yz2.p(this);
    }

    @Override
    public void e() {
        float f2;
        if (this.L()) {
            this.a(ro.e, 1.0f);
        }
        if (this.bu != this.bm) {
            ti ti2 = this.a(yj.d);
            ti2.b(bq);
            if (this.bm != null) {
                ti2.a(bq);
            }
        }
        this.bu = this.bm;
        if (!this.o.E && this.o.O().b("mobGriefing")) {
            int n2;
            int n3;
            aji aji2;
            if (this.cb().o() == awt.a) {
                int n4;
                if (this.Z.nextInt(20) == 0 && br[aji.b(aji2 = this.o.a(n4 = qh.c(this.s - 2.0 + this.Z.nextDouble() * 4.0), n3 = qh.c(this.t + this.Z.nextDouble() * 3.0), n2 = qh.c(this.u - 2.0 + this.Z.nextDouble() * 4.0)))]) {
                    this.a(aji2);
                    this.a(this.o.e(n4, n3, n2));
                    this.o.b(n4, n3, n2, ajn.a);
                }
            } else if (this.Z.nextInt(2000) == 0) {
                int n5 = qh.c(this.s - 1.0 + this.Z.nextDouble() * 2.0);
                n3 = qh.c(this.t + this.Z.nextDouble() * 2.0);
                n2 = qh.c(this.u - 1.0 + this.Z.nextDouble() * 2.0);
                aji2 = this.o.a(n5, n3, n2);
                aji aji3 = this.o.a(n5, n3 - 1, n2);
                if (aji2.o() == awt.a && aji3.o() != awt.a && aji3.d()) {
                    this.o.d(n5, n3, n2, this.cb(), this.cc(), 3);
                    this.a(ajn.a);
                }
            }
        }
        for (int i2 = 0; i2 < 2; ++i2) {
            this.o.a("portal", this.s + (this.Z.nextDouble() - 0.5) * (double)this.M, this.t + this.Z.nextDouble() * (double)this.N - 0.25, this.u + (this.Z.nextDouble() - 0.5) * (double)this.M, (this.Z.nextDouble() - 0.5) * 2.0, -this.Z.nextDouble(), (this.Z.nextDouble() - 0.5) * 2.0);
        }
        if (this.o.w() && !this.o.E && (f2 = this.d(1.0f)) > 0.5f && this.o.i(qh.c(this.s), qh.c(this.t), qh.c(this.u)) && this.Z.nextFloat() * 30.0f < (f2 - 0.4f) * 2.0f) {
            this.bm = null;
            this.a(false);
            this.bv = false;
            this.bZ();
        }
        if (this.L() || this.al()) {
            this.bm = null;
            this.a(false);
            this.bv = false;
            this.bZ();
        }
        if (this.cd() && !this.bv && this.Z.nextInt(100) == 0) {
            this.a(false);
        }
        this.bc = false;
        if (this.bm != null) {
            this.a(this.bm, 100.0f, 100.0f);
        }
        if (!this.o.E && this.Z()) {
            if (this.bm != null) {
                if (this.bm instanceof yz && this.f((yz)this.bm)) {
                    if (this.bm.f(this) < 16.0) {
                        this.bZ();
                    }
                    this.bs = 0;
                } else if (this.bm.f(this) > 256.0 && this.bs++ >= 30 && this.c(this.bm)) {
                    this.bs = 0;
                }
            } else {
                this.a(false);
                this.bs = 0;
            }
        }
        super.e();
    }

    protected boolean bZ() {
        double d2 = this.s + (this.Z.nextDouble() - 0.5) * 64.0;
        double d3 = this.t + (double)(this.Z.nextInt(64) - 32);
        double d4 = this.u + (this.Z.nextDouble() - 0.5) * 64.0;
        return this.k(d2, d3, d4);
    }

    protected boolean c(sa sa2) {
        azw azw2 = azw.a(this.s - sa2.s, this.C.b + (double)(this.N / 2.0f) - sa2.t + (double)sa2.g(), this.u - sa2.u);
        azw2 = azw2.a();
        double d2 = 16.0;
        double d3 = this.s + (this.Z.nextDouble() - 0.5) * 8.0 - azw2.a * d2;
        double d4 = this.t + (double)(this.Z.nextInt(16) - 8) - azw2.b * d2;
        double d5 = this.u + (this.Z.nextDouble() - 0.5) * 8.0 - azw2.c * d2;
        return this.k(d3, d4, d5);
    }

    protected boolean k(double d2, double d3, double d4) {
        int n2;
        int n3;
        int n4;
        double d5 = this.s;
        double d6 = this.t;
        double d7 = this.u;
        this.s = d2;
        this.t = d3;
        this.u = d4;
        boolean bl2 = false;
        int n5 = qh.c(this.s);
        if (this.o.d(n5, n4 = qh.c(this.t), n3 = qh.c(this.u))) {
            n2 = 0;
            while (n2 == 0 && n4 > 0) {
                aji aji2 = this.o.a(n5, n4 - 1, n3);
                if (aji2.o().c()) {
                    n2 = 1;
                    continue;
                }
                this.t -= 1.0;
                --n4;
            }
            if (n2 != 0) {
                this.b(this.s, this.t, this.u);
                if (this.o.a((sa)this, this.C).isEmpty() && !this.o.d(this.C)) {
                    bl2 = true;
                }
            }
        }
        if (bl2) {
            n2 = 128;
            for (int i2 = 0; i2 < n2; ++i2) {
                double d8 = (double)i2 / ((double)n2 - 1.0);
                float f2 = (this.Z.nextFloat() - 0.5f) * 0.2f;
                float f3 = (this.Z.nextFloat() - 0.5f) * 0.2f;
                float f4 = (this.Z.nextFloat() - 0.5f) * 0.2f;
                double d9 = d5 + (this.s - d5) * d8 + (this.Z.nextDouble() - 0.5) * (double)this.M * 2.0;
                double d10 = d6 + (this.t - d6) * d8 + this.Z.nextDouble() * (double)this.N;
                double d11 = d7 + (this.u - d7) * d8 + (this.Z.nextDouble() - 0.5) * (double)this.M * 2.0;
                this.o.a("portal", d9, d10, d11, (double)f2, (double)f3, f4);
            }
            this.o.a(d5, d6, d7, "mob.endermen.portal", 1.0f, 1.0f);
            this.a("mob.endermen.portal", 1.0f, 1.0f);
            return true;
        }
        this.b(d5, d6, d7);
        return false;
    }

    @Override
    protected String t() {
        return this.cd() ? "mob.endermen.scream" : "mob.endermen.idle";
    }

    @Override
    protected String aT() {
        return "mob.endermen.hit";
    }

    @Override
    protected String aU() {
        return "mob.endermen.death";
    }

    @Override
    protected adb u() {
        return ade.bi;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        adb adb2 = this.u();
        if (adb2 != null) {
            int n3 = this.Z.nextInt(2 + n2);
            for (int i2 = 0; i2 < n3; ++i2) {
                this.a(adb2, 1);
            }
        }
    }

    public void a(aji aji2) {
        this.af.b(16, (byte)(aji.b(aji2) & 0xFF));
    }

    public aji cb() {
        return aji.e(this.af.a(16));
    }

    public void a(int n2) {
        this.af.b(17, (byte)(n2 & 0xFF));
    }

    public int cc() {
        return this.af.a(17);
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        this.a(true);
        if (ro2 instanceof rp && ro2.j() instanceof yz) {
            this.bv = true;
        }
        if (ro2 instanceof rq) {
            this.bv = false;
            for (int i2 = 0; i2 < 64; ++i2) {
                if (!this.bZ()) continue;
                return true;
            }
            return false;
        }
        return super.a(ro2, f2);
    }

    public boolean cd() {
        return this.af.a(18) > 0;
    }

    public void a(boolean bl2) {
        this.af.b(18, (byte)(bl2 ? 1 : 0));
    }

    static {
        ya.br[aji.b((aji)ajn.c)] = true;
        ya.br[aji.b((aji)ajn.d)] = true;
        ya.br[aji.b((aji)ajn.m)] = true;
        ya.br[aji.b((aji)ajn.n)] = true;
        ya.br[aji.b((aji)ajn.N)] = true;
        ya.br[aji.b((aji)ajn.O)] = true;
        ya.br[aji.b((aji)ajn.P)] = true;
        ya.br[aji.b((aji)ajn.Q)] = true;
        ya.br[aji.b((aji)ajn.W)] = true;
        ya.br[aji.b((aji)ajn.aF)] = true;
        ya.br[aji.b((aji)ajn.aG)] = true;
        ya.br[aji.b((aji)ajn.aK)] = true;
        ya.br[aji.b((aji)ajn.ba)] = true;
        ya.br[aji.b((aji)ajn.bh)] = true;
    }
}

