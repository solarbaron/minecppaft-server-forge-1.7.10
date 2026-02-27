/*
 * Decompiled with CFR 0.152.
 */
public class zl
extends zk {
    public zl(ahb ahb2) {
        super(ahb2);
    }

    public zl(ahb ahb2, sv sv2) {
        super(ahb2, sv2);
    }

    public zl(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    protected void a(azu azu2) {
        int n2;
        if (azu2.g != null) {
            azu2.g.a(ro.a(this, (sa)this.j()), 0.0f);
        }
        if (!this.o.E && this.Z.nextInt(8) == 0) {
            n2 = 1;
            if (this.Z.nextInt(32) == 0) {
                n2 = 4;
            }
            for (int i2 = 0; i2 < n2; ++i2) {
                wg wg2 = new wg(this.o);
                wg2.c(-24000);
                wg2.b(this.s, this.t, this.u, this.y, 0.0f);
                this.o.d(wg2);
            }
        }
        for (n2 = 0; n2 < 8; ++n2) {
            this.o.a("snowballpoof", this.s, this.t, this.u, 0.0, 0.0, 0.0);
        }
        if (!this.o.E) {
            this.B();
        }
    }
}

