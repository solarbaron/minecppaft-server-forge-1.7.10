/*
 * Decompiled with CFR 0.152.
 */
public class zg
extends ze {
    public int e = 1;

    public zg(ahb ahb2) {
        super(ahb2);
    }

    public zg(ahb ahb2, sv sv2, double d2, double d3, double d4) {
        super(ahb2, sv2, d2, d3, d4);
    }

    @Override
    protected void a(azu azu2) {
        if (!this.o.E) {
            if (azu2.g != null) {
                azu2.g.a(ro.a(this, (sa)this.a), 6.0f);
            }
            this.o.a(null, this.s, this.t, this.u, (float)this.e, true, this.o.O().b("mobGriefing"));
            this.B();
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("ExplosionPower", this.e);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("ExplosionPower", 99)) {
            this.e = dh2.f("ExplosionPower");
        }
    }
}

