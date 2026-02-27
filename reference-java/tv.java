/*
 * Decompiled with CFR 0.152.
 */
public class tv {
    private sw a;
    private double b;
    private double c;
    private double d;
    private double e;
    private boolean f;

    public tv(sw sw2) {
        this.a = sw2;
        this.b = sw2.s;
        this.c = sw2.t;
        this.d = sw2.u;
    }

    public boolean a() {
        return this.f;
    }

    public double b() {
        return this.e;
    }

    public void a(double d2, double d3, double d4, double d5) {
        this.b = d2;
        this.c = d3;
        this.d = d4;
        this.e = d5;
        this.f = true;
    }

    public void c() {
        double d2;
        this.a.n(0.0f);
        if (!this.f) {
            return;
        }
        this.f = false;
        double d3 = this.b - this.a.s;
        int n2 = qh.c(this.a.C.b + 0.5);
        double d4 = this.c - (double)n2;
        double d5 = d3 * d3 + d4 * d4 + (d2 = this.d - this.a.u) * d2;
        if (d5 < 2.500000277905201E-7) {
            return;
        }
        float f2 = (float)(Math.atan2(d2, d3) * 180.0 / 3.1415927410125732) - 90.0f;
        this.a.y = this.a(this.a.y, f2, 30.0f);
        this.a.i((float)(this.e * this.a.a(yj.d).e()));
        if (d4 > 0.0 && d3 * d3 + d2 * d2 < 1.0) {
            this.a.l().a();
        }
    }

    private float a(float f2, float f3, float f4) {
        float f5 = qh.g(f3 - f2);
        if (f5 > f4) {
            f5 = f4;
        }
        if (f5 < -f4) {
            f5 = -f4;
        }
        return f2 + f5;
    }
}

