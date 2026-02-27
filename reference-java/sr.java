/*
 * Decompiled with CFR 0.152.
 */
public abstract class sr
extends sw {
    public sr(ahb ahb2) {
        super(ahb2);
    }

    @Override
    protected void b(float f2) {
    }

    @Override
    protected void a(double d2, boolean bl2) {
    }

    @Override
    public void e(float f2, float f3) {
        if (this.M()) {
            this.a(f2, f3, 0.02f);
            this.d(this.v, this.w, this.x);
            this.v *= (double)0.8f;
            this.w *= (double)0.8f;
            this.x *= (double)0.8f;
        } else if (this.P()) {
            this.a(f2, f3, 0.02f);
            this.d(this.v, this.w, this.x);
            this.v *= 0.5;
            this.w *= 0.5;
            this.x *= 0.5;
        } else {
            float f4 = 0.91f;
            if (this.D) {
                f4 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.91f;
            }
            float f5 = 0.16277136f / (f4 * f4 * f4);
            this.a(f2, f3, this.D ? 0.1f * f5 : 0.02f);
            f4 = 0.91f;
            if (this.D) {
                f4 = this.o.a((int)qh.c((double)this.s), (int)(qh.c((double)this.C.b) - 1), (int)qh.c((double)this.u)).K * 0.91f;
            }
            this.d(this.v, this.w, this.x);
            this.v *= (double)f4;
            this.w *= (double)f4;
            this.x *= (double)f4;
        }
        this.aE = this.aF;
        double d2 = this.s - this.p;
        double d3 = this.u - this.r;
        float f6 = qh.a(d2 * d2 + d3 * d3) * 4.0f;
        if (f6 > 1.0f) {
            f6 = 1.0f;
        }
        this.aF += (f6 - this.aF) * 0.4f;
        this.aG += this.aF;
    }

    @Override
    public boolean h_() {
        return false;
    }
}

