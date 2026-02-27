/*
 * Decompiled with CFR 0.152.
 */
public class tr {
    private sv a;
    private int b;
    private float c;

    public tr(sv sv2) {
        this.a = sv2;
    }

    public void a() {
        double d2 = this.a.s - this.a.p;
        double d3 = this.a.u - this.a.r;
        if (d2 * d2 + d3 * d3 > 2.500000277905201E-7) {
            this.a.aM = this.a.y;
            this.c = this.a.aO = this.a(this.a.aM, this.a.aO, 75.0f);
            this.b = 0;
            return;
        }
        float f2 = 75.0f;
        if (Math.abs(this.a.aO - this.c) > 15.0f) {
            this.b = 0;
            this.c = this.a.aO;
        } else {
            ++this.b;
            int n2 = 10;
            if (this.b > 10) {
                f2 = Math.max(1.0f - (float)(this.b - 10) / 10.0f, 0.0f) * 75.0f;
            }
        }
        this.a.aM = this.a(this.a.aO, this.a.aM, f2);
    }

    private float a(float f2, float f3, float f4) {
        float f5 = qh.g(f2 - f3);
        if (f5 < -f4) {
            f5 = -f4;
        }
        if (f5 >= f4) {
            f5 = f4;
        }
        return f2 - f5;
    }
}

