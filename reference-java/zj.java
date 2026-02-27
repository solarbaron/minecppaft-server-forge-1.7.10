/*
 * Decompiled with CFR 0.152.
 */
public class zj
extends zk {
    public zj(ahb ahb2) {
        super(ahb2);
    }

    public zj(ahb ahb2, sv sv2) {
        super(ahb2, sv2);
    }

    public zj(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    protected void a(azu azu2) {
        int n2;
        if (azu2.g != null) {
            n2 = 0;
            if (azu2.g instanceof xx) {
                n2 = 3;
            }
            azu2.g.a(ro.a(this, (sa)this.j()), (float)n2);
        }
        for (n2 = 0; n2 < 8; ++n2) {
            this.o.a("snowballpoof", this.s, this.t, this.u, 0.0, 0.0, 0.0);
        }
        if (!this.o.E) {
            this.B();
        }
    }
}

