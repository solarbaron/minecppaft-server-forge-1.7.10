/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

class bs {
    double a;
    double b;

    bs() {
    }

    bs(double d2, double d3) {
        this.a = d2;
        this.b = d3;
    }

    double a(bs bs2) {
        double d2 = this.a - bs2.a;
        double d3 = this.b - bs2.b;
        return Math.sqrt(d2 * d2 + d3 * d3);
    }

    void a() {
        double d2 = this.b();
        this.a /= d2;
        this.b /= d2;
    }

    float b() {
        return qh.a(this.a * this.a + this.b * this.b);
    }

    public void b(bs bs2) {
        this.a -= bs2.a;
        this.b -= bs2.b;
    }

    public boolean a(double d2, double d3, double d4, double d5) {
        boolean bl2 = false;
        if (this.a < d2) {
            this.a = d2;
            bl2 = true;
        } else if (this.a > d4) {
            this.a = d4;
            bl2 = true;
        }
        if (this.b < d3) {
            this.b = d3;
            bl2 = true;
        } else if (this.b > d5) {
            this.b = d5;
            bl2 = true;
        }
        return bl2;
    }

    public int a(ahb ahb2) {
        int n2 = qh.c(this.a);
        int n3 = qh.c(this.b);
        for (int i2 = 256; i2 > 0; --i2) {
            if (ahb2.a(n2, i2, n3).o() == awt.a) continue;
            return i2 + 1;
        }
        return 257;
    }

    public boolean b(ahb ahb2) {
        int n2 = qh.c(this.a);
        int n3 = qh.c(this.b);
        int n4 = 256;
        if (n4 > 0) {
            awt awt2 = ahb2.a(n2, n4, n3).o();
            return !awt2.d() && awt2 != awt.o;
        }
        return false;
    }

    public void a(Random random, double d2, double d3, double d4, double d5) {
        this.a = qh.a(random, d2, d4);
        this.b = qh.a(random, d3, d5);
    }
}

