/*
 * Decompiled with CFR 0.152.
 */
public class zn
extends zk {
    public zn(ahb ahb2) {
        super(ahb2);
    }

    public zn(ahb ahb2, sv sv2) {
        super(ahb2, sv2);
    }

    public zn(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    protected float i() {
        return 0.07f;
    }

    @Override
    protected float e() {
        return 0.7f;
    }

    @Override
    protected float f() {
        return -20.0f;
    }

    @Override
    protected void a(azu azu2) {
        if (!this.o.E) {
            int n2;
            this.o.c(2002, (int)Math.round(this.s), (int)Math.round(this.t), (int)Math.round(this.u), 0);
            for (int i2 = 3 + this.o.s.nextInt(5) + this.o.s.nextInt(5); i2 > 0; i2 -= n2) {
                n2 = sq.a(i2);
                this.o.d(new sq(this.o, this.s, this.t, this.u, n2));
            }
            this.B();
        }
    }
}

