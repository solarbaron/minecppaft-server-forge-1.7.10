/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public class xj
extends sa {
    private aji e;
    public int a;
    public int b;
    public boolean c = true;
    private boolean f;
    private boolean g;
    private int h = 40;
    private float i = 2.0f;
    public dh d;

    public xj(ahb ahb2) {
        super(ahb2);
    }

    public xj(ahb ahb2, double d2, double d3, double d4, aji aji2) {
        this(ahb2, d2, d3, d4, aji2, 0);
    }

    public xj(ahb ahb2, double d2, double d3, double d4, aji aji2, int n2) {
        super(ahb2);
        this.e = aji2;
        this.a = n2;
        this.k = true;
        this.a(0.98f, 0.98f);
        this.L = this.N / 2.0f;
        this.b(d2, d3, d4);
        this.v = 0.0;
        this.w = 0.0;
        this.x = 0.0;
        this.p = d2;
        this.q = d3;
        this.r = d4;
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    protected void c() {
    }

    @Override
    public boolean R() {
        return !this.K;
    }

    @Override
    public void h() {
        if (this.e.o() == awt.a) {
            this.B();
            return;
        }
        this.p = this.s;
        this.q = this.t;
        this.r = this.u;
        ++this.b;
        this.w -= (double)0.04f;
        this.d(this.v, this.w, this.x);
        this.v *= (double)0.98f;
        this.w *= (double)0.98f;
        this.x *= (double)0.98f;
        if (!this.o.E) {
            int n2 = qh.c(this.s);
            int n3 = qh.c(this.t);
            int n4 = qh.c(this.u);
            if (this.b == 1) {
                if (this.o.a(n2, n3, n4) == this.e) {
                    this.o.f(n2, n3, n4);
                } else {
                    this.B();
                    return;
                }
            }
            if (this.D) {
                this.v *= (double)0.7f;
                this.x *= (double)0.7f;
                this.w *= -0.5;
                if (this.o.a(n2, n3, n4) != ajn.M) {
                    this.B();
                    if (!this.f && this.o.a(this.e, n2, n3, n4, true, 1, null, null) && !akx.e(this.o, n2, n3 - 1, n4) && this.o.d(n2, n3, n4, this.e, this.a, 3)) {
                        aor aor2;
                        if (this.e instanceof akx) {
                            ((akx)this.e).a(this.o, n2, n3, n4, this.a);
                        }
                        if (this.d != null && this.e instanceof akw && (aor2 = this.o.o(n2, n3, n4)) != null) {
                            dh dh2 = new dh();
                            aor2.b(dh2);
                            for (String string : this.d.c()) {
                                dy dy2 = this.d.a(string);
                                if (string.equals("x") || string.equals("y") || string.equals("z")) continue;
                                dh2.a(string, dy2.b());
                            }
                            aor2.a(dh2);
                            aor2.e();
                        }
                    } else if (this.c && !this.f) {
                        this.a(new add(this.e, 1, this.e.a(this.a)), 0.0f);
                    }
                }
            } else if (this.b > 100 && !this.o.E && (n3 < 1 || n3 > 256) || this.b > 600) {
                if (this.c) {
                    this.a(new add(this.e, 1, this.e.a(this.a)), 0.0f);
                }
                this.B();
            }
        }
    }

    @Override
    protected void b(float f2) {
        int n2;
        if (this.g && (n2 = qh.f(f2 - 1.0f)) > 0) {
            ArrayList arrayList = new ArrayList(this.o.b((sa)this, this.C));
            boolean bl2 = this.e == ajn.bQ;
            ro ro2 = bl2 ? ro.m : ro.n;
            for (sa sa2 : arrayList) {
                sa2.a(ro2, (float)Math.min(qh.d((float)n2 * this.i), this.h));
            }
            if (bl2 && (double)this.Z.nextFloat() < (double)0.05f + (double)n2 * 0.05) {
                int n3 = this.a >> 2;
                int n4 = this.a & 3;
                if (++n3 > 2) {
                    this.f = true;
                } else {
                    this.a = n4 | n3 << 2;
                }
            }
        }
    }

    @Override
    protected void b(dh dh2) {
        dh2.a("Tile", (byte)aji.b(this.e));
        dh2.a("TileID", aji.b(this.e));
        dh2.a("Data", (byte)this.a);
        dh2.a("Time", (byte)this.b);
        dh2.a("DropItem", this.c);
        dh2.a("HurtEntities", this.g);
        dh2.a("FallHurtAmount", this.i);
        dh2.a("FallHurtMax", this.h);
        if (this.d != null) {
            dh2.a("TileEntityData", this.d);
        }
    }

    @Override
    protected void a(dh dh2) {
        this.e = dh2.b("TileID", 99) ? aji.e(dh2.f("TileID")) : aji.e(dh2.d("Tile") & 0xFF);
        this.a = dh2.d("Data") & 0xFF;
        this.b = dh2.d("Time") & 0xFF;
        if (dh2.b("HurtEntities", 99)) {
            this.g = dh2.n("HurtEntities");
            this.i = dh2.h("FallHurtAmount");
            this.h = dh2.f("FallHurtMax");
        } else if (this.e == ajn.bQ) {
            this.g = true;
        }
        if (dh2.b("DropItem", 99)) {
            this.c = dh2.n("DropItem");
        }
        if (dh2.b("TileEntityData", 10)) {
            this.d = dh2.m("TileEntityData");
        }
        if (this.e.o() == awt.a) {
            this.e = ajn.m;
        }
    }

    public void a(boolean bl2) {
        this.g = bl2;
    }

    @Override
    public void a(k k2) {
        super.a(k2);
        k2.a("Immitating block ID", aji.b(this.e));
        k2.a("Immitating block data", this.a);
    }

    public aji f() {
        return this.e;
    }
}

