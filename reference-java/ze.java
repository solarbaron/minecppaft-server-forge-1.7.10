/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public abstract class ze
extends sa {
    private int e = -1;
    private int f = -1;
    private int g = -1;
    private aji h;
    private boolean i;
    public sv a;
    private int at;
    private int au;
    public double b;
    public double c;
    public double d;

    public ze(ahb ahb2) {
        super(ahb2);
        this.a(1.0f, 1.0f);
    }

    @Override
    protected void c() {
    }

    public ze(ahb ahb2, double d2, double d3, double d4, double d5, double d6, double d7) {
        super(ahb2);
        this.a(1.0f, 1.0f);
        this.b(d2, d3, d4, this.y, this.z);
        this.b(d2, d3, d4);
        double d8 = qh.a(d5 * d5 + d6 * d6 + d7 * d7);
        this.b = d5 / d8 * 0.1;
        this.c = d6 / d8 * 0.1;
        this.d = d7 / d8 * 0.1;
    }

    public ze(ahb ahb2, sv sv2, double d2, double d3, double d4) {
        super(ahb2);
        this.a = sv2;
        this.a(1.0f, 1.0f);
        this.b(sv2.s, sv2.t, sv2.u, sv2.y, sv2.z);
        this.b(this.s, this.t, this.u);
        this.L = 0.0f;
        this.x = 0.0;
        this.w = 0.0;
        this.v = 0.0;
        double d5 = qh.a((d2 += this.Z.nextGaussian() * 0.4) * d2 + (d3 += this.Z.nextGaussian() * 0.4) * d3 + (d4 += this.Z.nextGaussian() * 0.4) * d4);
        this.b = d2 / d5 * 0.1;
        this.c = d3 / d5 * 0.1;
        this.d = d4 / d5 * 0.1;
    }

    @Override
    public void h() {
        if (!this.o.E && (this.a != null && this.a.K || !this.o.d((int)this.s, (int)this.t, (int)this.u))) {
            this.B();
            return;
        }
        super.h();
        this.e(1);
        if (this.i) {
            if (this.o.a(this.e, this.f, this.g) == this.h) {
                ++this.at;
                if (this.at == 600) {
                    this.B();
                }
                return;
            }
            this.i = false;
            this.v *= (double)(this.Z.nextFloat() * 0.2f);
            this.w *= (double)(this.Z.nextFloat() * 0.2f);
            this.x *= (double)(this.Z.nextFloat() * 0.2f);
            this.at = 0;
            this.au = 0;
        } else {
            ++this.au;
        }
        azw azw2 = azw.a(this.s, this.t, this.u);
        azw azw3 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        azu azu2 = this.o.a(azw2, azw3);
        azw2 = azw.a(this.s, this.t, this.u);
        azw3 = azw.a(this.s + this.v, this.t + this.w, this.u + this.x);
        if (azu2 != null) {
            azw3 = azw.a(azu2.f.a, azu2.f.b, azu2.f.c);
        }
        sa sa2 = null;
        List list = this.o.b((sa)this, this.C.a(this.v, this.w, this.x).b(1.0, 1.0, 1.0));
        double d2 = 0.0;
        for (int i2 = 0; i2 < list.size(); ++i2) {
            double d3;
            float f2;
            azt azt2;
            azu azu3;
            sa sa3 = (sa)list.get(i2);
            if (!sa3.R() || sa3.i(this.a) && this.au < 25 || (azu3 = (azt2 = sa3.C.b(f2 = 0.3f, f2, f2)).a(azw2, azw3)) == null || !((d3 = azw2.d(azu3.f)) < d2) && d2 != 0.0) continue;
            sa2 = sa3;
            d2 = d3;
        }
        if (sa2 != null) {
            azu2 = new azu(sa2);
        }
        if (azu2 != null) {
            this.a(azu2);
        }
        this.s += this.v;
        this.t += this.w;
        this.u += this.x;
        float f3 = qh.a(this.v * this.v + this.x * this.x);
        this.y = (float)(Math.atan2(this.x, this.v) * 180.0 / 3.1415927410125732) + 90.0f;
        this.z = (float)(Math.atan2(f3, this.w) * 180.0 / 3.1415927410125732) - 90.0f;
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
        float f4 = this.e();
        if (this.M()) {
            for (int i3 = 0; i3 < 4; ++i3) {
                float f5 = 0.25f;
                this.o.a("bubble", this.s - this.v * (double)f5, this.t - this.w * (double)f5, this.u - this.x * (double)f5, this.v, this.w, this.x);
            }
            f4 = 0.8f;
        }
        this.v += this.b;
        this.w += this.c;
        this.x += this.d;
        this.v *= (double)f4;
        this.w *= (double)f4;
        this.x *= (double)f4;
        this.o.a("smoke", this.s, this.t + 0.5, this.u, 0.0, 0.0, 0.0);
        this.b(this.s, this.t, this.u);
    }

    protected float e() {
        return 0.95f;
    }

    protected abstract void a(azu var1);

    @Override
    public void b(dh dh2) {
        dh2.a("xTile", (short)this.e);
        dh2.a("yTile", (short)this.f);
        dh2.a("zTile", (short)this.g);
        dh2.a("inTile", (byte)aji.b(this.h));
        dh2.a("inGround", (byte)(this.i ? 1 : 0));
        dh2.a("direction", this.a(this.v, this.w, this.x));
    }

    @Override
    public void a(dh dh2) {
        this.e = dh2.e("xTile");
        this.f = dh2.e("yTile");
        this.g = dh2.e("zTile");
        this.h = aji.e(dh2.d("inTile") & 0xFF);
        boolean bl2 = this.i = dh2.d("inGround") == 1;
        if (dh2.b("direction", 9)) {
            dq dq2 = dh2.c("direction", 6);
            this.v = dq2.d(0);
            this.w = dq2.d(1);
            this.x = dq2.d(2);
        } else {
            this.B();
        }
    }

    @Override
    public boolean R() {
        return true;
    }

    @Override
    public float af() {
        return 1.0f;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        if (this.aw()) {
            return false;
        }
        this.Q();
        if (ro2.j() != null) {
            azw azw2 = ro2.j().ag();
            if (azw2 != null) {
                this.v = azw2.a;
                this.w = azw2.b;
                this.x = azw2.c;
                this.b = this.v * 0.1;
                this.c = this.w * 0.1;
                this.d = this.x * 0.1;
            }
            if (ro2.j() instanceof sv) {
                this.a = (sv)ro2.j();
            }
            return true;
        }
        return false;
    }

    @Override
    public float d(float f2) {
        return 1.0f;
    }
}

