/*
 * Decompiled with CFR 0.152.
 */
public class tu {
    private sw a;
    private float b;
    private float c;
    private boolean d;
    private double e;
    private double f;
    private double g;

    public tu(sw sw2) {
        this.a = sw2;
    }

    public void a(sa sa2, float f2, float f3) {
        this.e = sa2.s;
        this.f = sa2 instanceof sv ? sa2.t + (double)sa2.g() : (sa2.C.b + sa2.C.e) / 2.0;
        this.g = sa2.u;
        this.b = f2;
        this.c = f3;
        this.d = true;
    }

    public void a(double d2, double d3, double d4, float f2, float f3) {
        this.e = d2;
        this.f = d3;
        this.g = d4;
        this.b = f2;
        this.c = f3;
        this.d = true;
    }

    public void a() {
        this.a.z = 0.0f;
        if (this.d) {
            this.d = false;
            double d2 = this.e - this.a.s;
            double d3 = this.f - (this.a.t + (double)this.a.g());
            double d4 = this.g - this.a.u;
            double d5 = qh.a(d2 * d2 + d4 * d4);
            float f2 = (float)(Math.atan2(d4, d2) * 180.0 / 3.1415927410125732) - 90.0f;
            float f3 = (float)(-(Math.atan2(d3, d5) * 180.0 / 3.1415927410125732));
            this.a.z = this.a(this.a.z, f3, this.c);
            this.a.aO = this.a(this.a.aO, f2, this.b);
        } else {
            this.a.aO = this.a(this.a.aO, this.a.aM, 10.0f);
        }
        float f4 = qh.g(this.a.aO - this.a.aM);
        if (!this.a.m().g()) {
            if (f4 < -75.0f) {
                this.a.aO = this.a.aM - 75.0f;
            }
            if (f4 > 75.0f) {
                this.a.aO = this.a.aM + 75.0f;
            }
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

