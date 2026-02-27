/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public abstract class zk
extends sa
implements zh {
    private int c = -1;
    private int d = -1;
    private int e = -1;
    private aji f;
    protected boolean a;
    public int b;
    private sv g;
    private String h;
    private int i;
    private int at;

    public zk(ahb ahb2) {
        super(ahb2);
        this.a(0.25f, 0.25f);
    }

    @Override
    protected void c() {
    }

    public zk(ahb ahb2, sv sv2) {
        super(ahb2);
        this.g = sv2;
        this.a(0.25f, 0.25f);
        this.b(sv2.s, sv2.t + (double)sv2.g(), sv2.u, sv2.y, sv2.z);
        this.s -= (double)(qh.b(this.y / 180.0f * (float)Math.PI) * 0.16f);
        this.t -= (double)0.1f;
        this.u -= (double)(qh.a(this.y / 180.0f * (float)Math.PI) * 0.16f);
        this.b(this.s, this.t, this.u);
        this.L = 0.0f;
        float f2 = 0.4f;
        this.v = -qh.a(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI) * f2;
        this.x = qh.b(this.y / 180.0f * (float)Math.PI) * qh.b(this.z / 180.0f * (float)Math.PI) * f2;
        this.w = -qh.a((this.z + this.f()) / 180.0f * (float)Math.PI) * f2;
        this.c(this.v, this.w, this.x, this.e(), 1.0f);
    }

    public zk(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2);
        this.i = 0;
        this.a(0.25f, 0.25f);
        this.b(d2, d3, d4);
        this.L = 0.0f;
    }

    protected float e() {
        return 1.5f;
    }

    protected float f() {
        return 0.0f;
    }

    @Override
    public void c(double d2, double d3, double d4, float f2, float f3) {
        float f4 = qh.a(d2 * d2 + d3 * d3 + d4 * d4);
        d2 /= (double)f4;
        d3 /= (double)f4;
        d4 /= (double)f4;
        d2 += this.Z.nextGaussian() * (double)0.0075f * (double)f3;
        d3 += this.Z.nextGaussian() * (double)0.0075f * (double)f3;
        d4 += this.Z.nextGaussian() * (double)0.0075f * (double)f3;
        this.v = d2 *= (double)f2;
        this.w = d3 *= (double)f2;
        this.x = d4 *= (double)f2;
        float f5 = qh.a(d2 * d2 + d4 * d4);
        this.A = this.y = (float)(Math.atan2(d2, d4) * 180.0 / 3.1415927410125732);
        this.B = this.z = (float)(Math.atan2(d3, f5) * 180.0 / 3.1415927410125732);
        this.i = 0;
    }

    @Override
    public void h() {
        this.S = this.s;
        this.T = this.t;
        this.U = this.u;
        super.h();
        if (this.b > 0) {
            --this.b;
        }
        if (this.a) {
            if (this.o.a(this.c, this.d, this.e) == this.f) {
                ++this.i;
                if (this.i == 1200) {
                    this.B();
                }
                return;
            }
            this.a = false;
            this.v *= (double)(this.Z.nextFloat() * 0.2f);
            this.w *= (double)(this.Z.nextFloat() * 0.2f);
            this.x *= (double)(this.Z.nextFloat() * 0.2f);
            this.i = 0;
            this.at = 0;
        } else {
            ++this.at;
        }
        azw azw2 = azw.a(this.s, this.t, this.u);
        azw azw3 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        azu azu2 = this.o.a(azw2, azw3);
        azw2 = azw.a(this.s, this.t, this.u);
        azw3 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        if (azu2 != null) {
            azw3 = azw.a(azu2.f.a, azu2.f.b, azu2.f.c);
        }
        if (!this.o.E) {
            sa sa2 = null;
            List list = this.o.b((sa)this, this.C.a(this.v, this.w, this.x).b(1.0, 1.0, 1.0));
            double d2 = 0.0;
            sv sv2 = this.j();
            for (int i2 = 0; i2 < list.size(); ++i2) {
                double d3;
                float f2;
                azt azt2;
                azu azu3;
                sa sa3 = (sa)list.get(i2);
                if (!sa3.R() || sa3 == sv2 && this.at < 5 || (azu3 = (azt2 = sa3.C.b(f2 = 0.3f, f2, f2)).a(azw2, azw3)) == null || !((d3 = azw2.d(azu3.f)) < d2) && d2 != 0.0) continue;
                sa2 = sa3;
                d2 = d3;
            }
            if (sa2 != null) {
                azu2 = new azu(sa2);
            }
        }
        if (azu2 != null) {
            if (azu2.a == azv.b && this.o.a(azu2.b, azu2.c, azu2.d) == ajn.aO) {
                this.ah();
            } else {
                this.a(azu2);
            }
        }
        this.s += this.v;
        this.t += this.w;
        this.u += this.x;
        float f3 = qh.a(this.v * this.v + this.x * this.x);
        this.y = (float)(Math.atan2(this.v, this.x) * 180.0 / 3.1415927410125732);
        this.z = (float)(Math.atan2(this.w, f3) * 180.0 / 3.1415927410125732);
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
        float f4 = 0.99f;
        float f5 = this.i();
        if (this.M()) {
            for (int i3 = 0; i3 < 4; ++i3) {
                float f6 = 0.25f;
                this.o.a("bubble", this.s - this.v * (double)f6, this.t - this.w * (double)f6, this.u - this.x * (double)f6, this.v, this.w, this.x);
            }
            f4 = 0.8f;
        }
        this.v *= (double)f4;
        this.w *= (double)f4;
        this.x *= (double)f4;
        this.w -= (double)f5;
        this.b(this.s, this.t, this.u);
    }

    protected float i() {
        return 0.03f;
    }

    protected abstract void a(azu var1);

    @Override
    public void b(dh dh2) {
        dh2.a("xTile", (short)this.c);
        dh2.a("yTile", (short)this.d);
        dh2.a("zTile", (short)this.e);
        dh2.a("inTile", (byte)aji.b(this.f));
        dh2.a("shake", (byte)this.b);
        dh2.a("inGround", (byte)(this.a ? 1 : 0));
        if ((this.h == null || this.h.length() == 0) && this.g != null && this.g instanceof yz) {
            this.h = this.g.b_();
        }
        dh2.a("ownerName", this.h == null ? "" : this.h);
    }

    @Override
    public void a(dh dh2) {
        this.c = dh2.e("xTile");
        this.d = dh2.e("yTile");
        this.e = dh2.e("zTile");
        this.f = aji.e(dh2.d("inTile") & 0xFF);
        this.b = dh2.d("shake") & 0xFF;
        this.a = dh2.d("inGround") == 1;
        this.h = dh2.j("ownerName");
        if (this.h != null && this.h.length() == 0) {
            this.h = null;
        }
    }

    public sv j() {
        if (this.g == null && this.h != null && this.h.length() > 0) {
            this.g = this.o.a(this.h);
        }
        return this.g;
    }
}

