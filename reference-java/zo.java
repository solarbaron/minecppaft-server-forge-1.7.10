/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class zo
extends zk {
    private add c;

    public zo(ahb ahb2) {
        super(ahb2);
    }

    public zo(ahb ahb2, sv sv2, int n2) {
        this(ahb2, sv2, new add(ade.bn, 1, n2));
    }

    public zo(ahb ahb2, sv sv2, add add2) {
        super(ahb2, sv2);
        this.c = add2;
    }

    public zo(ahb ahb2, double d2, double d3, double d4, add add2) {
        super(ahb2, d2, d3, d4);
        this.c = add2;
    }

    @Override
    protected float i() {
        return 0.05f;
    }

    @Override
    protected float e() {
        return 0.5f;
    }

    @Override
    protected float f() {
        return -20.0f;
    }

    public void a(int n2) {
        if (this.c == null) {
            this.c = new add(ade.bn, 1, 0);
        }
        this.c.b(n2);
    }

    public int k() {
        if (this.c == null) {
            this.c = new add(ade.bn, 1, 0);
        }
        return this.c.k();
    }

    @Override
    protected void a(azu azu2) {
        if (!this.o.E) {
            azt azt2;
            List list;
            List list2 = ade.bn.g(this.c);
            if (list2 != null && !list2.isEmpty() && (list = this.o.a(sv.class, azt2 = this.C.b(4.0, 2.0, 4.0))) != null && !list.isEmpty()) {
                for (sv sv2 : list) {
                    double d2 = this.f(sv2);
                    if (!(d2 < 16.0)) continue;
                    double d3 = 1.0 - Math.sqrt(d2) / 4.0;
                    if (sv2 == azu2.g) {
                        d3 = 1.0;
                    }
                    for (rw rw2 : list2) {
                        int n2 = rw2.a();
                        if (rv.a[n2].b()) {
                            rv.a[n2].a(this.j(), sv2, rw2.c(), d3);
                            continue;
                        }
                        int n3 = (int)(d3 * (double)rw2.b() + 0.5);
                        if (n3 <= 20) continue;
                        sv2.c(new rw(n2, n3, rw2.c()));
                    }
                }
            }
            this.o.c(2002, (int)Math.round(this.s), (int)Math.round(this.t), (int)Math.round(this.u), this.k());
            this.B();
        }
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        if (dh2.b("Potion", 10)) {
            this.c = add.a(dh2.m("Potion"));
        } else {
            this.a(dh2.f("potionValue"));
        }
        if (this.c == null) {
            this.B();
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        if (this.c != null) {
            dh2.a("Potion", this.c.b(new dh()));
        }
    }
}

