/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class zc
extends sa
implements zh {
    private int d = -1;
    private int e = -1;
    private int f = -1;
    private aji g;
    private int h;
    private boolean i;
    public int a;
    public int b;
    public sa c;
    private int at;
    private int au;
    private double av = 2.0;
    private int aw;

    public zc(ahb ahb2) {
        super(ahb2);
        this.j = 10.0;
        this.a(0.5f, 0.5f);
    }

    public zc(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2);
        this.j = 10.0;
        this.a(0.5f, 0.5f);
        this.b(d2, d3, d4);
        this.L = 0.0f;
    }

    public zc(ahb ahb2, sv sv2, sv sv3, float f2, float f3) {
        super(ahb2);
        this.j = 10.0;
        this.c = sv2;
        if (sv2 instanceof yz) {
            this.a = 1;
        }
        this.t = sv2.t + (double)sv2.g() - (double)0.1f;
        double d2 = sv3.s - sv2.s;
        double d3 = sv3.C.b + (double)(sv3.N / 3.0f) - this.t;
        double d4 = sv3.u - sv2.u;
        double d5 = qh.a(d2 * d2 + d4 * d4);
        if (d5 < 1.0E-7) {
            return;
        }
        float f4 = (float)(Math.atan2(d4, d2) * 180.0 / 3.1415927410125732) - 90.0f;
        float f5 = (float)(-(Math.atan2(d3, d5) * 180.0 / 3.1415927410125732));
        double d6 = d2 / d5;
        double d7 = d4 / d5;
        this.b(sv2.s + d6, this.t, sv2.u + d7, f4, f5);
        this.L = 0.0f;
        float f6 = (float)d5 * 0.2f;
        this.c(d2, d3 + (double)f6, d4, f2, f3);
    }

    public zc(ahb ahb2, sv sv2, float f2) {
        super(ahb2);
        this.j = 10.0;
        this.c = sv2;
        if (sv2 instanceof yz) {
            this.a = 1;
        }
        this.a(0.5f, 0.5f);
        this.b(sv2.s, sv2.t + (double)sv2.g(), sv2.u, sv2.y, sv2.z);
        this.s -= (double)(qh.b(this.y / 180.0f * (float)Math.PI) * 0.16f);
        this.t -= (double)0.1f;
        this.u -= (double)(qh.a(this.y / 180.0f * (float)Math.PI) * 0.16f);
        this.b(this.s, this.t, this.u);
        this.L = 0.0f;
        this.v = -qh.a(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI);
        this.x = qh.b(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI);
        this.w = -qh.a(this.z / 180.0f * (float)Math.PI);
        this.c(this.v, this.w, this.x, f2 * 1.5f, 1.0f);
    }

    @Override
    protected void c() {
        this.af.a(16, (Object)0);
    }

    @Override
    public void c(double d2, double d3, double d4, float f2, float f3) {
        float f4 = qh.a(d2 * d2 + d3 * d3 + d4 * d4);
        d2 /= (double)f4;
        d3 /= (double)f4;
        d4 /= (double)f4;
        d2 += this.Z.nextGaussian() * (double)(this.Z.nextBoolean() ? -1 : 1) * (double)0.0075f * (double)f3;
        d3 += this.Z.nextGaussian() * (double)(this.Z.nextBoolean() ? -1 : 1) * (double)0.0075f * (double)f3;
        d4 += this.Z.nextGaussian() * (double)(this.Z.nextBoolean() ? -1 : 1) * (double)0.0075f * (double)f3;
        this.v = d2 *= (double)f2;
        this.w = d3 *= (double)f2;
        this.x = d4 *= (double)f2;
        float f5 = qh.a(d2 * d2 + d4 * d4);
        this.A = this.y = (float)(Math.atan2(d2, d4) * 180.0 / 3.1415927410125732);
        this.B = this.z = (float)(Math.atan2(d3, f5) * 180.0 / 3.1415927410125732);
        this.at = 0;
    }

    @Override
    public void h() {
        Object object;
        int n2;
        Object object2;
        aji aji2;
        super.h();
        if (this.B == 0.0f && this.A == 0.0f) {
            float f2 = qh.a(this.v * this.v + this.x * this.x);
            this.A = this.y = (float)(Math.atan2(this.v, this.x) * 180.0 / 3.1415927410125732);
            this.B = this.z = (float)(Math.atan2(this.w, f2) * 180.0 / 3.1415927410125732);
        }
        if ((aji2 = this.o.a(this.d, this.e, this.f)).o() != awt.a) {
            aji2.a((ahl)this.o, this.d, this.e, this.f);
            object2 = aji2.a(this.o, this.d, this.e, this.f);
            if (object2 != null && ((azt)object2).a(azw.a(this.s, this.t, this.u))) {
                this.i = true;
            }
        }
        if (this.b > 0) {
            --this.b;
        }
        if (this.i) {
            int n3 = this.o.e(this.d, this.e, this.f);
            if (aji2 != this.g || n3 != this.h) {
                this.i = false;
                this.v *= (double)(this.Z.nextFloat() * 0.2f);
                this.w *= (double)(this.Z.nextFloat() * 0.2f);
                this.x *= (double)(this.Z.nextFloat() * 0.2f);
                this.at = 0;
                this.au = 0;
                return;
            }
            ++this.at;
            if (this.at == 1200) {
                this.B();
            }
            return;
        }
        ++this.au;
        object2 = azw.a(this.s, this.t, this.u);
        azw azw2 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        azu azu2 = this.o.a((azw)object2, azw2, false, true, false);
        object2 = azw.a(this.s, this.t, this.u);
        azw2 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        if (azu2 != null) {
            azw2 = azw.a(azu2.f.a, azu2.f.b, azu2.f.c);
        }
        sa sa2 = null;
        List list = this.o.b((sa)this, this.C.a(this.v, this.w, this.x).b(1.0, 1.0, 1.0));
        double d2 = 0.0;
        for (n2 = 0; n2 < list.size(); ++n2) {
            double d3;
            float f3;
            azu azu3;
            sa sa3 = (sa)list.get(n2);
            if (!sa3.R() || sa3 == this.c && this.au < 5 || (azu3 = ((azt)(object = sa3.C.b(f3 = 0.3f, f3, f3))).a((azw)object2, azw2)) == null || !((d3 = ((azw)object2).d(azu3.f)) < d2) && d2 != 0.0) continue;
            sa2 = sa3;
            d2 = d3;
        }
        if (sa2 != null) {
            azu2 = new azu(sa2);
        }
        if (azu2 != null && azu2.g != null && azu2.g instanceof yz) {
            yz yz2 = (yz)azu2.g;
            if (yz2.bE.a || this.c instanceof yz && !((yz)this.c).a(yz2)) {
                azu2 = null;
            }
        }
        if (azu2 != null) {
            if (azu2.g != null) {
                float f4 = qh.a(this.v * this.v + this.w * this.w + this.x * this.x);
                int n4 = qh.f((double)f4 * this.av);
                if (this.f()) {
                    n4 += this.Z.nextInt(n4 / 2 + 2);
                }
                ro ro2 = null;
                ro2 = this.c == null ? ro.a(this, (sa)this) : ro.a(this, this.c);
                if (this.al() && !(azu2.g instanceof ya)) {
                    azu2.g.e(5);
                }
                if (azu2.g.a(ro2, (float)n4)) {
                    if (azu2.g instanceof sv) {
                        float f5;
                        object = (sv)azu2.g;
                        if (!this.o.E) {
                            ((sv)object).p(((sv)object).aZ() + 1);
                        }
                        if (this.aw > 0 && (f5 = qh.a(this.v * this.v + this.x * this.x)) > 0.0f) {
                            azu2.g.g(this.v * (double)this.aw * (double)0.6f / (double)f5, 0.1, this.x * (double)this.aw * (double)0.6f / (double)f5);
                        }
                        if (this.c != null && this.c instanceof sv) {
                            afv.a((sv)object, this.c);
                            afv.b((sv)this.c, (sa)object);
                        }
                        if (this.c != null && azu2.g != this.c && azu2.g instanceof yz && this.c instanceof mw) {
                            ((mw)this.c).a.a(new gv(6, 0.0f));
                        }
                    }
                    this.a("random.bowhit", 1.0f, 1.2f / (this.Z.nextFloat() * 0.2f + 0.9f));
                    if (!(azu2.g instanceof ya)) {
                        this.B();
                    }
                } else {
                    this.v *= (double)-0.1f;
                    this.w *= (double)-0.1f;
                    this.x *= (double)-0.1f;
                    this.y += 180.0f;
                    this.A += 180.0f;
                    this.au = 0;
                }
            } else {
                this.d = azu2.b;
                this.e = azu2.c;
                this.f = azu2.d;
                this.g = this.o.a(this.d, this.e, this.f);
                this.h = this.o.e(this.d, this.e, this.f);
                this.v = (float)(azu2.f.a - this.s);
                this.w = (float)(azu2.f.b - this.t);
                this.x = (float)(azu2.f.c - this.u);
                float f6 = qh.a(this.v * this.v + this.w * this.w + this.x * this.x);
                this.s -= this.v / (double)f6 * (double)0.05f;
                this.t -= this.w / (double)f6 * (double)0.05f;
                this.u -= this.x / (double)f6 * (double)0.05f;
                this.a("random.bowhit", 1.0f, 1.2f / (this.Z.nextFloat() * 0.2f + 0.9f));
                this.i = true;
                this.b = 7;
                this.a(false);
                if (this.g.o() != awt.a) {
                    this.g.a(this.o, this.d, this.e, this.f, this);
                }
            }
        }
        if (this.f()) {
            for (n2 = 0; n2 < 4; ++n2) {
                this.o.a("crit", this.s + this.v * (double)n2 / 4.0, this.t + this.w * (double)n2 / 4.0, this.u + this.x * (double)n2 / 4.0, -this.v, -this.w + 0.2, -this.x);
            }
        }
        this.s += this.v;
        this.t += this.w;
        this.u += this.x;
        float f7 = qh.a(this.v * this.v + this.x * this.x);
        this.y = (float)(Math.atan2(this.v, this.x) * 180.0 / 3.1415927410125732);
        this.z = (float)(Math.atan2(this.w, f7) * 180.0 / 3.1415927410125732);
        while (this.z - this.B < -180.0f) {
            this.B -= 360.0f;
        }
        while (this.z - this.B >= 180.0f) {
            this.B += 360.0f;
        }
        while (this.y - this.A < -180.0f) {
            this.A -= 360.0f;
        }
        while (this.y - this.A >= 180.0f) {
            this.A += 360.0f;
        }
        this.z = this.B + (this.z - this.B) * 0.2f;
        this.y = this.A + (this.y - this.A) * 0.2f;
        float f8 = 0.99f;
        float f9 = 0.05f;
        if (this.M()) {
            for (int i2 = 0; i2 < 4; ++i2) {
                float f10 = 0.25f;
                this.o.a("bubble", this.s - this.v * (double)f10, this.t - this.w * (double)f10, this.u - this.x * (double)f10, this.v, this.w, this.x);
            }
            f8 = 0.8f;
        }
        if (this.L()) {
            this.F();
        }
        this.v *= (double)f8;
        this.w *= (double)f8;
        this.x *= (double)f8;
        this.w -= (double)f9;
        this.b(this.s, this.t, this.u);
        this.I();
    }

    @Override
    public void b(dh dh2) {
        dh2.a("xTile", (short)this.d);
        dh2.a("yTile", (short)this.e);
        dh2.a("zTile", (short)this.f);
        dh2.a("life", (short)this.at);
        dh2.a("inTile", (byte)aji.b(this.g));
        dh2.a("inData", (byte)this.h);
        dh2.a("shake", (byte)this.b);
        dh2.a("inGround", (byte)(this.i ? 1 : 0));
        dh2.a("pickup", (byte)this.a);
        dh2.a("damage", this.av);
    }

    @Override
    public void a(dh dh2) {
        this.d = dh2.e("xTile");
        this.e = dh2.e("yTile");
        this.f = dh2.e("zTile");
        this.at = dh2.e("life");
        this.g = aji.e(dh2.d("inTile") & 0xFF);
        this.h = dh2.d("inData") & 0xFF;
        this.b = dh2.d("shake") & 0xFF;
        boolean bl2 = this.i = dh2.d("inGround") == 1;
        if (dh2.b("damage", 99)) {
            this.av = dh2.i("damage");
        }
        if (dh2.b("pickup", 99)) {
            this.a = dh2.d("pickup");
        } else if (dh2.b("player", 99)) {
            this.a = dh2.n("player") ? 1 : 0;
        }
    }

    @Override
    public void b_(yz yz2) {
        boolean bl2;
        if (this.o.E || !this.i || this.b > 0) {
            return;
        }
        boolean bl3 = bl2 = this.a == 1 || this.a == 2 && yz2.bE.d;
        if (this.a == 1 && !yz2.bm.a(new add(ade.g, 1))) {
            bl2 = false;
        }
        if (bl2) {
            this.a("random.pop", 0.2f, ((this.Z.nextFloat() - this.Z.nextFloat()) * 0.7f + 1.0f) * 2.0f);
            yz2.a((sa)this, 1);
            this.B();
        }
    }

    @Override
    protected boolean g_() {
        return false;
    }

    public void b(double d2) {
        this.av = d2;
    }

    public double e() {
        return this.av;
    }

    public void a(int n2) {
        this.aw = n2;
    }

    @Override
    public boolean av() {
        return false;
    }

    public void a(boolean bl2) {
        byte by2 = this.af.a(16);
        if (bl2) {
            this.af.b(16, (byte)(by2 | 1));
        } else {
            this.af.b(16, (byte)(by2 & 0xFFFFFFFE));
        }
    }

    public boolean f() {
        byte by2 = this.af.a(16);
        return (by2 & 1) != 0;
    }
}

