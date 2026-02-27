/*
 * Decompiled with CFR 0.152.
 */
public class zi
extends ze {
    public zi(ahb ahb2) {
        super(ahb2);
        this.a(0.3125f, 0.3125f);
    }

    public zi(ahb ahb2, sv sv2, double d2, double d3, double d4) {
        super(ahb2, sv2, d2, d3, d4);
        this.a(0.3125f, 0.3125f);
    }

    public zi(ahb ahb2, double d2, double d3, double d4, double d5, double d6, double d7) {
        super(ahb2, d2, d3, d4, d5, d6, d7);
        this.a(0.3125f, 0.3125f);
    }

    @Override
    protected void a(azu azu2) {
        if (!this.o.E) {
            if (azu2.g != null) {
                if (!azu2.g.K() && azu2.g.a(ro.a(this, (sa)this.a), 5.0f)) {
                    azu2.g.e(5);
                }
            } else {
                int n2 = azu2.b;
                int n3 = azu2.c;
                int n4 = azu2.d;
                switch (azu2.e) {
                    case 1: {
                        ++n3;
                        break;
                    }
                    case 0: {
                        --n3;
                        break;
                    }
                    case 2: {
                        --n4;
                        break;
                    }
                    case 3: {
                        ++n4;
                        break;
                    }
                    case 5: {
                        ++n2;
                        break;
                    }
                    case 4: {
                        --n2;
                    }
                }
                if (this.o.c(n2, n3, n4)) {
                    this.o.b(n2, n3, n4, ajn.ab);
                }
            }
            this.B();
        }
    }

    @Override
    public boolean R() {
        return false;
    }

    @Override
    public boolean a(ro ro2, float f2) {
        return false;
    }
}

