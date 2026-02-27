/*
 * Decompiled with CFR 0.152.
 */
public class zf
extends sa {
    private int a;
    private int b;

    public zf(ahb ahb2) {
        super(ahb2);
        this.a(0.25f, 0.25f);
    }

    @Override
    protected void c() {
        this.af.a(8, 5);
    }

    public zf(ahb ahb2, double d2, double d3, double d4, add add2) {
        super(ahb2);
        this.a = 0;
        this.a(0.25f, 0.25f);
        this.b(d2, d3, d4);
        this.L = 0.0f;
        int n2 = 1;
        if (add2 != null && add2.p()) {
            this.af.b(8, add2);
            dh dh2 = add2.q();
            dh dh3 = dh2.m("Fireworks");
            if (dh3 != null) {
                n2 += dh3.d("Flight");
            }
        }
        this.v = this.Z.nextGaussian() * 0.001;
        this.x = this.Z.nextGaussian() * 0.001;
        this.w = 0.05;
        this.b = 10 * n2 + this.Z.nextInt(6) + this.Z.nextInt(7);
    }

    @Override
    public void h() {
        this.S = this.s;
        this.T = this.t;
        this.U = this.u;
        super.h();
        this.v *= 1.15;
        this.x *= 1.15;
        this.w += 0.04;
        this.d(this.v, this.w, this.x);
        float f2 = qh.a(this.v * this.v + this.x * this.x);
        this.y = (float)(Math.atan2(this.v, this.x) * 180.0 / 3.1415927410125732);
        this.z = (float)(Math.atan2(this.w, f2) * 180.0 / 3.1415927410125732);
        while (this.z - this.B < -180.0f) {
            this.B -= 360.0f;
        }
        while (this.z - this.B >= 180.0f) {
            this.B += 360.0f;
        }
        while (this.y - this.A < -180.0f) {
            this.A -= 360.0f;
        }
        while (this.y - this.A >= 180.0f) {
            this.A += 360.0f;
        }
        this.z = this.B + (this.z - this.B) * 0.2f;
        this.y = this.A + (this.y - this.A) * 0.2f;
        if (this.a == 0) {
            this.o.a(this, "fireworks.launch", 3.0f, 1.0f);
        }
        ++this.a;
        if (this.o.E && this.a % 2 < 2) {
            this.o.a("fireworksSpark", this.s, this.t - 0.3, this.u, this.Z.nextGaussian() * 0.05, -this.w * 0.5, this.Z.nextGaussian() * 0.05);
        }
        if (!this.o.E && this.a > this.b) {
            this.o.a((sa)this, (byte)17);
            this.B();
        }
    }

    @Override
    public void b(dh dh2) {
        dh2.a("Life", this.a);
        dh2.a("LifeTime", this.b);
        add add2 = this.af.f(8);
        if (add2 != null) {
            dh dh3 = new dh();
            add2.b(dh3);
            dh2.a("FireworksItem", dh3);
        }
    }

    @Override
    public void a(dh dh2) {
        add add2;
        this.a = dh2.f("Life");
        this.b = dh2.f("LifeTime");
        dh dh3 = dh2.m("FireworksItem");
        if (dh3 != null && (add2 = add.a(dh3)) != null) {
            this.af.b(8, add2);
        }
    }

    @Override
    public float d(float f2) {
        return super.d(f2);
    }

    @Override
    public boolean av() {
        return false;
    }
}

