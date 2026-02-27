/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class su
extends ss {
    public su(ahb ahb2) {
        super(ahb2);
    }

    public su(ahb ahb2, int n2, int n3, int n4) {
        super(ahb2, n2, n3, n4, 0);
        this.b((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5);
    }

    @Override
    protected void c() {
        super.c();
    }

    @Override
    public void a(int n2) {
    }

    @Override
    public int f() {
        return 9;
    }

    @Override
    public int i() {
        return 9;
    }

    @Override
    public void b(sa sa2) {
    }

    @Override
    public boolean d(dh dh2) {
        return false;
    }

    @Override
    public void b(dh dh2) {
    }

    @Override
    public void a(dh dh2) {
    }

    @Override
    public boolean c(yz yz2) {
        double d2;
        List list;
        add add2 = yz2.be();
        boolean bl2 = false;
        if (add2 != null && add2.b() == ade.ca && !this.o.E && (list = this.o.a(sw.class, azt.a(this.s - (d2 = 7.0), this.t - d2, this.u - d2, this.s + d2, this.t + d2, this.u + d2))) != null) {
            for (sw sw2 : list) {
                if (!sw2.bN() || sw2.bO() != yz2) continue;
                sw2.b((sa)this, true);
                bl2 = true;
            }
        }
        if (!this.o.E && !bl2) {
            this.B();
            if (yz2.bE.d && (list = this.o.a(sw.class, azt.a(this.s - (d2 = 7.0), this.t - d2, this.u - d2, this.s + d2, this.t + d2, this.u + d2))) != null) {
                for (sw sw2 : list) {
                    if (!sw2.bN() || sw2.bO() != this) continue;
                    sw2.a(true, false);
                }
            }
        }
        return true;
    }

    @Override
    public boolean e() {
        return this.o.a(this.b, this.c, this.d).b() == 11;
    }

    public static su a(ahb ahb2, int n2, int n3, int n4) {
        su su2 = new su(ahb2, n2, n3, n4);
        su2.n = true;
        ahb2.d(su2);
        return su2;
    }

    public static su b(ahb ahb2, int n2, int n3, int n4) {
        List list = ahb2.a(su.class, azt.a((double)n2 - 1.0, (double)n3 - 1.0, (double)n4 - 1.0, (double)n2 + 1.0, (double)n3 + 1.0, (double)n4 + 1.0));
        if (list != null) {
            for (su su2 : list) {
                if (su2.b != n2 || su2.c != n3 || su2.d != n4) continue;
                return su2;
            }
        }
        return null;
    }
}

