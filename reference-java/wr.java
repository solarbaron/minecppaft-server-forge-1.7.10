/*
 * Decompiled with CFR 0.152.
 */
public class wr
extends wl
implements yi {
    public wr(ahb ahb2) {
        super(ahb2);
        this.a(0.4f, 1.8f);
        this.m().a(true);
        this.c.a(1, new vd(this, 1.25, 20, 10.0f));
        this.c.a(2, new vc(this, 1.0));
        this.c.a(3, new un(this, yz.class, 6.0f));
        this.c.a(4, new vb(this));
        this.d.a(1, new vo(this, sw.class, 0, true, false, yb.a));
    }

    @Override
    public boolean bk() {
        return true;
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(4.0);
        this.a(yj.d).a(0.2f);
    }

    @Override
    public void e() {
        super.e();
        int n2 = qh.c(this.s);
        int n3 = qh.c(this.t);
        int n4 = qh.c(this.u);
        if (this.L()) {
            this.a(ro.e, 1.0f);
        }
        if (this.o.a(n2, n4).a(n2, n3, n4) > 1.0f) {
            this.a(ro.b, 1.0f);
        }
        for (int i2 = 0; i2 < 4; ++i2) {
            n2 = qh.c(this.s + (double)((float)(i2 % 2 * 2 - 1) * 0.25f));
            if (this.o.a(n2, n3 = qh.c(this.t), n4 = qh.c(this.u + (double)((float)(i2 / 2 % 2 * 2 - 1) * 0.25f))).o() != awt.a || !(this.o.a(n2, n4).a(n2, n3, n4) < 0.8f) || !ajn.aC.c(this.o, n2, n3, n4)) continue;
            this.o.b(n2, n3, n4, ajn.aC);
        }
    }

    @Override
    protected adb u() {
        return ade.ay;
    }

    @Override
    protected void b(boolean bl2, int n2) {
        int n3 = this.Z.nextInt(16);
        for (int i2 = 0; i2 < n3; ++i2) {
            this.a(ade.ay, 1);
        }
    }

    @Override
    public void a(sv sv2, float f2) {
        zj zj2 = new zj(this.o, this);
        double d2 = sv2.s - this.s;
        double d3 = sv2.t + (double)sv2.g() - (double)1.1f - zj2.t;
        double d4 = sv2.u - this.u;
        float f3 = qh.a(d2 * d2 + d4 * d4) * 0.2f;
        zj2.c(d2, d3 + (double)f3, d4, 1.6f, 12.0f);
        this.a("random.bow", 1.0f, 1.0f / (this.aI().nextFloat() * 0.4f + 0.8f));
        this.o.d(zj2);
    }
}

