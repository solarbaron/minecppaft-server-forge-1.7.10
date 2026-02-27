/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class vn
extends vu {
    boolean a;
    private int b;

    public vn(td td2, boolean bl2) {
        super(td2, false);
        this.a = bl2;
        this.a(1);
    }

    @Override
    public boolean a() {
        int n2 = this.c.aK();
        return n2 != this.b && this.a(this.c.aJ(), false);
    }

    @Override
    public void c() {
        this.c.d(this.c.aJ());
        this.b = this.c.aK();
        if (this.a) {
            double d2 = this.f();
            List list = this.c.o.a(this.c.getClass(), azt.a(this.c.s, this.c.t, this.c.u, this.c.s + 1.0, this.c.t + 1.0, this.c.u + 1.0).b(d2, 10.0, d2));
            for (td td2 : list) {
                if (this.c == td2 || td2.o() != null || td2.c(this.c.aJ())) continue;
                td2.d(this.c.aJ());
            }
        }
        super.c();
    }
}

