/*
 * Decompiled with CFR 0.152.
 */
public class zr {
    private int a = 20;
    private float b = 5.0f;
    private float c;
    private int d;
    private int e = 20;

    public void a(int n2, float f2) {
        this.a = Math.min(n2 + this.a, 20);
        this.b = Math.min(this.b + (float)n2 * f2 * 2.0f, (float)this.a);
    }

    public void a(acx acx2, add add2) {
        this.a(acx2.g(add2), acx2.h(add2));
    }

    public void a(yz yz2) {
        rd rd2 = yz2.o.r;
        this.e = this.a;
        if (this.c > 4.0f) {
            this.c -= 4.0f;
            if (this.b > 0.0f) {
                this.b = Math.max(this.b - 1.0f, 0.0f);
            } else if (rd2 != rd.a) {
                this.a = Math.max(this.a - 1, 0);
            }
        }
        if (yz2.o.O().b("naturalRegeneration") && this.a >= 18 && yz2.bR()) {
            ++this.d;
            if (this.d >= 80) {
                yz2.f(1.0f);
                this.a(3.0f);
                this.d = 0;
            }
        } else if (this.a <= 0) {
            ++this.d;
            if (this.d >= 80) {
                if (yz2.aS() > 10.0f || rd2 == rd.d || yz2.aS() > 1.0f && rd2 == rd.c) {
                    yz2.a(ro.f, 1.0f);
                }
                this.d = 0;
            }
        } else {
            this.d = 0;
        }
    }

    public void a(dh dh2) {
        if (dh2.b("foodLevel", 99)) {
            this.a = dh2.f("foodLevel");
            this.d = dh2.f("foodTickTimer");
            this.b = dh2.h("foodSaturationLevel");
            this.c = dh2.h("foodExhaustionLevel");
        }
    }

    public void b(dh dh2) {
        dh2.a("foodLevel", this.a);
        dh2.a("foodTickTimer", this.d);
        dh2.a("foodSaturationLevel", this.b);
        dh2.a("foodExhaustionLevel", this.c);
    }

    public int a() {
        return this.a;
    }

    public boolean c() {
        return this.a < 20;
    }

    public void a(float f2) {
        this.c = Math.min(this.c + f2, 40.0f);
    }

    public float e() {
        return this.b;
    }
}

