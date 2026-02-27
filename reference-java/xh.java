/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class xh
extends xg {
    private int b;
    public long a;
    private int c;

    public xh(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2);
        this.b(d2, d3, d4, 0.0f, 0.0f);
        this.b = 2;
        this.a = this.Z.nextLong();
        this.c = this.Z.nextInt(3) + 1;
        if (!ahb2.E && ahb2.O().b("doFireTick") && (ahb2.r == rd.c || ahb2.r == rd.d) && ahb2.a(qh.c(d2), qh.c(d3), qh.c(d4), 10)) {
            int n2;
            int n3;
            int n4 = qh.c(d2);
            if (ahb2.a(n4, n3 = qh.c(d3), n2 = qh.c(d4)).o() == awt.a && ajn.ab.c(ahb2, n4, n3, n2)) {
                ahb2.b(n4, n3, n2, ajn.ab);
            }
            for (n4 = 0; n4 < 4; ++n4) {
                int n5;
                n3 = qh.c(d2) + this.Z.nextInt(3) - 1;
                if (ahb2.a(n3, n2 = qh.c(d3) + this.Z.nextInt(3) - 1, n5 = qh.c(d4) + this.Z.nextInt(3) - 1).o() != awt.a || !ajn.ab.c(ahb2, n3, n2, n5)) continue;
                ahb2.b(n3, n2, n5, ajn.ab);
            }
        }
    }

    @Override
    public void h() {
        super.h();
        if (this.b == 2) {
            this.o.a(this.s, this.t, this.u, "ambient.weather.thunder", 10000.0f, 0.8f + this.Z.nextFloat() * 0.2f);
            this.o.a(this.s, this.t, this.u, "random.explode", 2.0f, 0.5f + this.Z.nextFloat() * 0.2f);
        }
        --this.b;
        if (this.b < 0) {
            if (this.c == 0) {
                this.B();
            } else if (this.b < -this.Z.nextInt(10)) {
                int n2;
                int n3;
                int n4;
                --this.c;
                this.b = 1;
                this.a = this.Z.nextLong();
                if (!this.o.E && this.o.O().b("doFireTick") && this.o.a(qh.c(this.s), qh.c(this.t), qh.c(this.u), 10) && this.o.a(n4 = qh.c(this.s), n3 = qh.c(this.t), n2 = qh.c(this.u)).o() == awt.a && ajn.ab.c(this.o, n4, n3, n2)) {
                    this.o.b(n4, n3, n2, ajn.ab);
                }
            }
        }
        if (this.b >= 0) {
            if (this.o.E) {
                this.o.q = 2;
            } else {
                double d2 = 3.0;
                List list = this.o.b((sa)this, azt.a(this.s - d2, this.t - d2, this.u - d2, this.s + d2, this.t + 6.0 + d2, this.u + d2));
                for (int i2 = 0; i2 < list.size(); ++i2) {
                    sa sa2 = (sa)list.get(i2);
                    sa2.a(this);
                }
            }
        }
    }

    @Override
    protected void c() {
    }

    @Override
    protected void a(dh dh2) {
    }

    @Override
    protected void b(dh dh2) {
    }
}

