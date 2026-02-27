/*
 * Decompiled with CFR 0.152.
 */
public class azt {
    public double a;
    public double b;
    public double c;
    public double d;
    public double e;
    public double f;

    public static azt a(double d2, double d3, double d4, double d5, double d6, double d7) {
        return new azt(d2, d3, d4, d5, d6, d7);
    }

    protected azt(double d2, double d3, double d4, double d5, double d6, double d7) {
        this.a = d2;
        this.b = d3;
        this.c = d4;
        this.d = d5;
        this.e = d6;
        this.f = d7;
    }

    public azt b(double d2, double d3, double d4, double d5, double d6, double d7) {
        this.a = d2;
        this.b = d3;
        this.c = d4;
        this.d = d5;
        this.e = d6;
        this.f = d7;
        return this;
    }

    public azt a(double d2, double d3, double d4) {
        double d5 = this.a;
        double d6 = this.b;
        double d7 = this.c;
        double d8 = this.d;
        double d9 = this.e;
        double d10 = this.f;
        if (d2 < 0.0) {
            d5 += d2;
        }
        if (d2 > 0.0) {
            d8 += d2;
        }
        if (d3 < 0.0) {
            d6 += d3;
        }
        if (d3 > 0.0) {
            d9 += d3;
        }
        if (d4 < 0.0) {
            d7 += d4;
        }
        if (d4 > 0.0) {
            d10 += d4;
        }
        return azt.a(d5, d6, d7, d8, d9, d10);
    }

    public azt b(double d2, double d3, double d4) {
        double d5 = this.a - d2;
        double d6 = this.b - d3;
        double d7 = this.c - d4;
        double d8 = this.d + d2;
        double d9 = this.e + d3;
        double d10 = this.f + d4;
        return azt.a(d5, d6, d7, d8, d9, d10);
    }

    public azt a(azt azt2) {
        double d2 = Math.min(this.a, azt2.a);
        double d3 = Math.min(this.b, azt2.b);
        double d4 = Math.min(this.c, azt2.c);
        double d5 = Math.max(this.d, azt2.d);
        double d6 = Math.max(this.e, azt2.e);
        double d7 = Math.max(this.f, azt2.f);
        return azt.a(d2, d3, d4, d5, d6, d7);
    }

    public azt c(double d2, double d3, double d4) {
        return azt.a(this.a + d2, this.b + d3, this.c + d4, this.d + d2, this.e + d3, this.f + d4);
    }

    public double a(azt azt2, double d2) {
        double d3;
        if (azt2.e <= this.b || azt2.b >= this.e) {
            return d2;
        }
        if (azt2.f <= this.c || azt2.c >= this.f) {
            return d2;
        }
        if (d2 > 0.0 && azt2.d <= this.a && (d3 = this.a - azt2.d) < d2) {
            d2 = d3;
        }
        if (d2 < 0.0 && azt2.a >= this.d && (d3 = this.d - azt2.a) > d2) {
            d2 = d3;
        }
        return d2;
    }

    public double b(azt azt2, double d2) {
        double d3;
        if (azt2.d <= this.a || azt2.a >= this.d) {
            return d2;
        }
        if (azt2.f <= this.c || azt2.c >= this.f) {
            return d2;
        }
        if (d2 > 0.0 && azt2.e <= this.b && (d3 = this.b - azt2.e) < d2) {
            d2 = d3;
        }
        if (d2 < 0.0 && azt2.b >= this.e && (d3 = this.e - azt2.b) > d2) {
            d2 = d3;
        }
        return d2;
    }

    public double c(azt azt2, double d2) {
        double d3;
        if (azt2.d <= this.a || azt2.a >= this.d) {
            return d2;
        }
        if (azt2.e <= this.b || azt2.b >= this.e) {
            return d2;
        }
        if (d2 > 0.0 && azt2.f <= this.c && (d3 = this.c - azt2.f) < d2) {
            d2 = d3;
        }
        if (d2 < 0.0 && azt2.c >= this.f && (d3 = this.f - azt2.c) > d2) {
            d2 = d3;
        }
        return d2;
    }

    public boolean b(azt azt2) {
        if (azt2.d <= this.a || azt2.a >= this.d) {
            return false;
        }
        if (azt2.e <= this.b || azt2.b >= this.e) {
            return false;
        }
        return !(azt2.f <= this.c) && !(azt2.c >= this.f);
    }

    public azt d(double d2, double d3, double d4) {
        this.a += d2;
        this.b += d3;
        this.c += d4;
        this.d += d2;
        this.e += d3;
        this.f += d4;
        return this;
    }

    public boolean a(azw azw2) {
        if (azw2.a <= this.a || azw2.a >= this.d) {
            return false;
        }
        if (azw2.b <= this.b || azw2.b >= this.e) {
            return false;
        }
        return !(azw2.c <= this.c) && !(azw2.c >= this.f);
    }

    public double a() {
        double d2 = this.d - this.a;
        double d3 = this.e - this.b;
        double d4 = this.f - this.c;
        return (d2 + d3 + d4) / 3.0;
    }

    public azt e(double d2, double d3, double d4) {
        double d5 = this.a + d2;
        double d6 = this.b + d3;
        double d7 = this.c + d4;
        double d8 = this.d - d2;
        double d9 = this.e - d3;
        double d10 = this.f - d4;
        return azt.a(d5, d6, d7, d8, d9, d10);
    }

    public azt b() {
        return azt.a(this.a, this.b, this.c, this.d, this.e, this.f);
    }

    public azu a(azw azw2, azw azw3) {
        azw azw4 = azw2.b(azw3, this.a);
        azw azw5 = azw2.b(azw3, this.d);
        azw azw6 = azw2.c(azw3, this.b);
        azw azw7 = azw2.c(azw3, this.e);
        azw azw8 = azw2.d(azw3, this.c);
        azw azw9 = azw2.d(azw3, this.f);
        if (!this.b(azw4)) {
            azw4 = null;
        }
        if (!this.b(azw5)) {
            azw5 = null;
        }
        if (!this.c(azw6)) {
            azw6 = null;
        }
        if (!this.c(azw7)) {
            azw7 = null;
        }
        if (!this.d(azw8)) {
            azw8 = null;
        }
        if (!this.d(azw9)) {
            azw9 = null;
        }
        azw azw10 = null;
        if (azw4 != null && (azw10 == null || azw2.e(azw4) < azw2.e(azw10))) {
            azw10 = azw4;
        }
        if (azw5 != null && (azw10 == null || azw2.e(azw5) < azw2.e(azw10))) {
            azw10 = azw5;
        }
        if (azw6 != null && (azw10 == null || azw2.e(azw6) < azw2.e(azw10))) {
            azw10 = azw6;
        }
        if (azw7 != null && (azw10 == null || azw2.e(azw7) < azw2.e(azw10))) {
            azw10 = azw7;
        }
        if (azw8 != null && (azw10 == null || azw2.e(azw8) < azw2.e(azw10))) {
            azw10 = azw8;
        }
        if (azw9 != null && (azw10 == null || azw2.e(azw9) < azw2.e(azw10))) {
            azw10 = azw9;
        }
        if (azw10 == null) {
            return null;
        }
        int n2 = -1;
        if (azw10 == azw4) {
            n2 = 4;
        }
        if (azw10 == azw5) {
            n2 = 5;
        }
        if (azw10 == azw6) {
            n2 = 0;
        }
        if (azw10 == azw7) {
            n2 = 1;
        }
        if (azw10 == azw8) {
            n2 = 2;
        }
        if (azw10 == azw9) {
            n2 = 3;
        }
        return new azu(0, 0, 0, n2, azw10);
    }

    private boolean b(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.b >= this.b && azw2.b <= this.e && azw2.c >= this.c && azw2.c <= this.f;
    }

    private boolean c(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.a >= this.a && azw2.a <= this.d && azw2.c >= this.c && azw2.c <= this.f;
    }

    private boolean d(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.a >= this.a && azw2.a <= this.d && azw2.b >= this.b && azw2.b <= this.e;
    }

    public void d(azt azt2) {
        this.a = azt2.a;
        this.b = azt2.b;
        this.c = azt2.c;
        this.d = azt2.d;
        this.e = azt2.e;
        this.f = azt2.f;
    }

    public String toString() {
        return "box[" + this.a + ", " + this.b + ", " + this.c + " -> " + this.d + ", " + this.e + ", " + this.f + "]";
    }
}

