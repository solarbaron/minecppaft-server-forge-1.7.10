/*
 * Decompiled with CFR 0.152.
 */
public class um
extends ui {
    sw a;
    sv b;
    float c;

    public um(sw sw2, float f2) {
        this.a = sw2;
        this.c = f2;
        this.a(5);
    }

    @Override
    public boolean a() {
        this.b = this.a.o();
        if (this.b == null) {
            return false;
        }
        double d2 = this.a.f(this.b);
        if (d2 < 4.0 || d2 > 16.0) {
            return false;
        }
        if (!this.a.D) {
            return false;
        }
        return this.a.aI().nextInt(5) == 0;
    }

    @Override
    public boolean b() {
        return !this.a.D;
    }

    @Override
    public void c() {
        double d2 = this.b.s - this.a.s;
        double d3 = this.b.u - this.a.u;
        float f2 = qh.a(d2 * d2 + d3 * d3);
        this.a.v += d2 / (double)f2 * 0.5 * (double)0.8f + this.a.v * (double)0.2f;
        this.a.x += d3 / (double)f2 * 0.5 * (double)0.8f + this.a.x * (double)0.2f;
        this.a.w = this.c;
    }
}

