/*
 * Decompiled with CFR 0.152.
 */
public class zm
extends zk {
    public zm(ahb ahb2) {
        super(ahb2);
    }

    public zm(ahb ahb2, sv sv2) {
        super(ahb2, sv2);
    }

    @Override
    protected void a(azu azu2) {
        if (azu2.g != null) {
            azu2.g.a(ro.a(this, (sa)this.j()), 0.0f);
        }
        for (int i2 = 0; i2 < 32; ++i2) {
            this.o.a("portal", this.s, this.t + this.Z.nextDouble() * 2.0, this.u, this.Z.nextGaussian(), 0.0, this.Z.nextGaussian());
        }
        if (!this.o.E) {
            if (this.j() != null && this.j() instanceof mw) {
                mw mw2 = (mw)this.j();
                if (mw2.a.b().d() && mw2.o == this.o) {
                    if (this.j().am()) {
                        this.j().a((sa)null);
                    }
                    this.j().a(this.s, this.t, this.u);
                    this.j().R = 0.0f;
                    this.j().a(ro.h, 5.0f);
                }
            }
            this.B();
        }
    }
}

