/*
 * Decompiled with CFR 0.152.
 */
public class ws
extends wu {
    public float bp;
    public float bq;
    public float br;
    public float bs;
    public float bt;
    public float bu;
    public float bv;
    public float bw;
    private float bx;
    private float by;
    private float bz;
    private float bA;
    private float bB;
    private float bC;

    public ws(ahb ahb2) {
        super(ahb2);
        this.a(0.95f, 0.95f);
        this.by = 1.0f / (this.Z.nextFloat() + 1.0f) * 0.2f;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(10.0);
    }

    @Override
    protected String t() {
        return null;
    }

    @Override
    protected String aT() {
        return null;
    }

    @Override
    protected String aU() {
        return null;
    }

    @Override
    protected float bf() {
        return 0.4f;
    }

    @Override
    protected adb u() {
        return adb.d(0);
    }

    @Override
    protected boolean g_() {
        return false;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3 = this.Z.nextInt(3 + n2) + 1;
        for (int i2 = 0; i2 < n3; ++i2) {
            this.a(new add(ade.aR, 1, 0), 0.0f);
        }
    }

    @Override
    public boolean M() {
        return this.o.a(this.C.b(0.0, -0.6f, 0.0), awt.h, (sa)this);
    }

    @Override
    public void e() {
        super.e();
        this.bq = this.bp;
        this.bs = this.br;
        this.bu = this.bt;
        this.bw = this.bv;
        this.bt += this.by;
        if (this.bt > (float)Math.PI * 2) {
            this.bt -= (float)Math.PI * 2;
            if (this.Z.nextInt(10) == 0) {
                this.by = 1.0f / (this.Z.nextFloat() + 1.0f) * 0.2f;
            }
        }
        if (this.M()) {
            float f2;
            if (this.bt < (float)Math.PI) {
                f2 = this.bt / (float)Math.PI;
                this.bv = qh.a(f2 * f2 * (float)Math.PI) * (float)Math.PI * 0.25f;
                if ((double)f2 > 0.75) {
                    this.bx = 1.0f;
                    this.bz = 1.0f;
                } else {
                    this.bz *= 0.8f;
                }
            } else {
                this.bv = 0.0f;
                this.bx *= 0.9f;
                this.bz *= 0.99f;
            }
            if (!this.o.E) {
                this.v = this.bA * this.bx;
                this.w = this.bB * this.bx;
                this.x = this.bC * this.bx;
            }
            f2 = qh.a(this.v * this.v + this.x * this.x);
            this.aM += (-((float)Math.atan2(this.v, this.x)) * 180.0f / (float)Math.PI - this.aM) * 0.1f;
            this.y = this.aM;
            this.br += (float)Math.PI * this.bz * 1.5f;
            this.bp += (-((float)Math.atan2(f2, this.w)) * 180.0f / (float)Math.PI - this.bp) * 0.1f;
        } else {
            this.bv = qh.e(qh.a(this.bt)) * (float)Math.PI * 0.25f;
            if (!this.o.E) {
                this.v = 0.0;
                this.w -= 0.08;
                this.w *= (double)0.98f;
                this.x = 0.0;
            }
            this.bp = (float)((double)this.bp + (double)(-90.0f - this.bp) * 0.02);
        }
    }

    @Override
    public void e(float f2, float f3) {
        this.d(this.v, this.w, this.x);
    }

    @Override
    protected void bq() {
        ++this.aU;
        if (this.aU > 100) {
            this.bC = 0.0f;
            this.bB = 0.0f;
            this.bA = 0.0f;
        } else if (this.Z.nextInt(50) == 0 || !this.ac || this.bA == 0.0f && this.bB == 0.0f && this.bC == 0.0f) {
            float f2 = this.Z.nextFloat() * (float)Math.PI * 2.0f;
            this.bA = qh.b(f2) * 0.2f;
            this.bB = -0.1f + this.Z.nextFloat() * 0.2f;
            this.bC = qh.a(f2) * 0.2f;
        }
        this.w();
    }

    @Override
    public boolean by() {
        return this.t > 45.0 && this.t < 63.0 && super.by();
    }
}

