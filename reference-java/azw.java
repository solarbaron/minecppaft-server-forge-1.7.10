/*
 * Decompiled with CFR 0.152.
 */
public class azw {
    public double a;
    public double b;
    public double c;

    public static azw a(double d2, double d3, double d4) {
        return new azw(d2, d3, d4);
    }

    protected azw(double d2, double d3, double d4) {
        if (d2 == -0.0) {
            d2 = 0.0;
        }
        if (d3 == -0.0) {
            d3 = 0.0;
        }
        if (d4 == -0.0) {
            d4 = 0.0;
        }
        this.a = d2;
        this.b = d3;
        this.c = d4;
    }

    protected azw b(double d2, double d3, double d4) {
        this.a = d2;
        this.b = d3;
        this.c = d4;
        return this;
    }

    public azw a() {
        double d2 = qh.a(this.a * this.a + this.b * this.b + this.c * this.c);
        if (d2 < 1.0E-4) {
            return azw.a(0.0, 0.0, 0.0);
        }
        return azw.a(this.a / d2, this.b / d2, this.c / d2);
    }

    public double b(azw azw2) {
        return this.a * azw2.a + this.b * azw2.b + this.c * azw2.c;
    }

    public azw c(double d2, double d3, double d4) {
        return azw.a(this.a + d2, this.b + d3, this.c + d4);
    }

    public double d(azw azw2) {
        double d2 = azw2.a - this.a;
        double d3 = azw2.b - this.b;
        double d4 = azw2.c - this.c;
        return qh.a(d2 * d2 + d3 * d3 + d4 * d4);
    }

    public double e(azw azw2) {
        double d2 = azw2.a - this.a;
        double d3 = azw2.b - this.b;
        double d4 = azw2.c - this.c;
        return d2 * d2 + d3 * d3 + d4 * d4;
    }

    public double d(double d2, double d3, double d4) {
        double d5 = d2 - this.a;
        double d6 = d3 - this.b;
        double d7 = d4 - this.c;
        return d5 * d5 + d6 * d6 + d7 * d7;
    }

    public double b() {
        return qh.a(this.a * this.a + this.b * this.b + this.c * this.c);
    }

    public azw b(azw azw2, double d2) {
        double d3 = azw2.a - this.a;
        double d4 = azw2.b - this.b;
        double d5 = azw2.c - this.c;
        if (d3 * d3 < (double)1.0E-7f) {
            return null;
        }
        double d6 = (d2 - this.a) / d3;
        if (d6 < 0.0 || d6 > 1.0) {
            return null;
        }
        return azw.a(this.a + d3 * d6, this.b + d4 * d6, this.c + d5 * d6);
    }

    public azw c(azw azw2, double d2) {
        double d3 = azw2.a - this.a;
        double d4 = azw2.b - this.b;
        double d5 = azw2.c - this.c;
        if (d4 * d4 < (double)1.0E-7f) {
            return null;
        }
        double d6 = (d2 - this.b) / d4;
        if (d6 < 0.0 || d6 > 1.0) {
            return null;
        }
        return azw.a(this.a + d3 * d6, this.b + d4 * d6, this.c + d5 * d6);
    }

    public azw d(azw azw2, double d2) {
        double d3 = azw2.a - this.a;
        double d4 = azw2.b - this.b;
        double d5 = azw2.c - this.c;
        if (d5 * d5 < (double)1.0E-7f) {
            return null;
        }
        double d6 = (d2 - this.c) / d5;
        if (d6 < 0.0 || d6 > 1.0) {
            return null;
        }
        return azw.a(this.a + d3 * d6, this.b + d4 * d6, this.c + d5 * d6);
    }

    public String toString() {
        return "(" + this.a + ", " + this.b + ", " + this.c + ")";
    }

    public void a(float f2) {
        float f3 = qh.b(f2);
        float f4 = qh.a(f2);
        double d2 = this.a;
        double d3 = this.b * (double)f3 + this.c * (double)f4;
        double d4 = this.c * (double)f3 - this.b * (double)f4;
        this.b(d2, d3, d4);
    }

    public void b(float f2) {
        float f3 = qh.b(f2);
        float f4 = qh.a(f2);
        double d2 = this.a * (double)f3 + this.c * (double)f4;
        double d3 = this.b;
        double d4 = this.c * (double)f3 - this.a * (double)f4;
        this.b(d2, d3, d4);
    }
}

