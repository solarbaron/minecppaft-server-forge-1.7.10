/*
 * Decompiled with CFR 0.152.
 */
public class zd
extends sa {
    private double a;
    private double b;
    private double c;
    private int d;
    private boolean e;

    public zd(ahb ahb2) {
        super(ahb2);
        this.a(0.25f, 0.25f);
    }

    @Override
    protected void c() {
    }

    public zd(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2);
        this.d = 0;
        this.a(0.25f, 0.25f);
        this.b(d2, d3, d4);
        this.L = 0.0f;
    }

    public void a(double d2, int n2, double d3) {
        double d4 = d2 - this.s;
        double d5 = d3 - this.u;
        float f2 = qh.a(d4 * d4 + d5 * d5);
        if (f2 > 12.0f) {
            this.a = this.s + d4 / (double)f2 * 12.0;
            this.c = this.u + d5 / (double)f2 * 12.0;
            this.b = this.t + 8.0;
        } else {
            this.a = d2;
            this.b = n2;
            this.c = d3;
        }
        this.d = 0;
        this.e = this.Z.nextInt(5) > 0;
    }

    @Override
    public void h() {
        this.S = this.s;
        this.T = this.t;
        this.U = this.u;
        super.h();
        this.s += this.v;
        this.t += this.w;
        this.u += this.x;
        float f2 = qh.a(this.v * this.v + this.x * this.x);
        this.y = (float)(Math.atan2(this.v, this.x) * 180.0 / 3.1415927410125732);
        this.z = (float)(Math.atan2(this.w, f2) * 180.0 / 3.1415927410125732);
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
        if (!this.o.E) {
            double d2 = this.a - this.s;
            double d3 = this.c - this.u;
            float f3 = (float)Math.sqrt(d2 * d2 + d3 * d3);
            float f4 = (float)Math.atan2(d3, d2);
            double d4 = (double)f2 + (double)(f3 - f2) * 0.0025;
            if (f3 < 1.0f) {
                d4 *= 0.8;
                this.w *= 0.8;
            }
            this.v = Math.cos(f4) * d4;
            this.x = Math.sin(f4) * d4;
            this.w = this.t < this.b ? (this.w += (1.0 - this.w) * (double)0.015f) : (this.w += (-1.0 - this.w) * (double)0.015f);
        }
        float f5 = 0.25f;
        if (this.M()) {
            for (int i2 = 0; i2 < 4; ++i2) {
                this.o.a("bubble", this.s - this.v * (double)f5, this.t - this.w * (double)f5, this.u - this.x * (double)f5, this.v, this.w, this.x);
            }
        } else {
            this.o.a("portal", this.s - this.v * (double)f5 + this.Z.nextDouble() * 0.6 - 0.3, this.t - this.w * (double)f5 - 0.5, this.u - this.x * (double)f5 + this.Z.nextDouble() * 0.6 - 0.3, this.v, this.w, this.x);
        }
        if (!this.o.E) {
            this.b(this.s, this.t, this.u);
            ++this.d;
            if (this.d > 80 && !this.o.E) {
                this.B();
                if (this.e) {
                    this.o.d(new xk(this.o, this.s, this.t, this.u, new add(ade.bv)));
                } else {
                    this.o.c(2003, (int)Math.round(this.s), (int)Math.round(this.t), (int)Math.round(this.u), 0);
                }
            }
        }
    }

    @Override
    public void b(dh dh2) {
    }

    @Override
    public void a(dh dh2) {
    }

    @Override
    public float d(float f2) {
        return 1.0f;
    }

    @Override
    public boolean av() {
        return false;
    }
}

