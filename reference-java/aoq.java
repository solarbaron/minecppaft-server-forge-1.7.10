/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class aoq
extends aor
implements rb {
    public static final rv[][] a = new rv[][]{{rv.c, rv.e}, {rv.m, rv.j}, {rv.g}, {rv.l}};
    private boolean k;
    private int l = -1;
    private int m;
    private int n;
    private add o;
    private String p;

    @Override
    public void h() {
        if (this.b.I() % 80L == 0L) {
            this.y();
            this.x();
        }
    }

    private void x() {
        if (this.k && this.l > 0 && !this.b.E && this.m > 0) {
            double d2 = this.l * 10 + 10;
            int n2 = 0;
            if (this.l >= 4 && this.m == this.n) {
                n2 = 1;
            }
            azt azt2 = azt.a(this.c, this.d, this.e, this.c + 1, this.d + 1, this.e + 1).b(d2, d2, d2);
            azt2.e = this.b.R();
            List list = this.b.a(yz.class, azt2);
            for (yz yz2 : list) {
                yz2.c(new rw(this.m, 180, n2, true));
            }
            if (this.l >= 4 && this.m != this.n && this.n > 0) {
                for (yz yz2 : list) {
                    yz2.c(new rw(this.n, 180, 0, true));
                }
            }
        }
    }

    private void y() {
        int n2 = this.l;
        if (!this.b.i(this.c, this.d + 1, this.e)) {
            this.k = false;
            this.l = 0;
        } else {
            int n3;
            this.k = true;
            this.l = 0;
            int n4 = 1;
            while (n4 <= 4 && (n3 = this.d - n4) >= 0) {
                boolean bl2 = true;
                block1: for (int i2 = this.c - n4; i2 <= this.c + n4 && bl2; ++i2) {
                    for (int i3 = this.e - n4; i3 <= this.e + n4; ++i3) {
                        aji aji2 = this.b.a(i2, n3, i3);
                        if (aji2 == ajn.bE || aji2 == ajn.R || aji2 == ajn.ah || aji2 == ajn.S) continue;
                        bl2 = false;
                        continue block1;
                    }
                }
                if (!bl2) break;
                this.l = n4++;
            }
            if (this.l == 0) {
                this.k = false;
            }
        }
        if (!this.b.E && this.l == 4 && n2 < this.l) {
            for (yz yz2 : this.b.a(yz.class, azt.a(this.c, this.d, this.e, this.c, this.d - 4, this.e).b(10.0, 5.0, 10.0))) {
                yz2.a(pc.K);
            }
        }
    }

    public int j() {
        return this.m;
    }

    public int k() {
        return this.n;
    }

    public int l() {
        return this.l;
    }

    public void d(int n2) {
        this.m = 0;
        for (int i2 = 0; i2 < this.l && i2 < 3; ++i2) {
            for (rv rv2 : a[i2]) {
                if (rv2.H != n2) continue;
                this.m = n2;
                return;
            }
        }
    }

    public void e(int n2) {
        this.n = 0;
        if (this.l >= 4) {
            for (int i2 = 0; i2 < 4; ++i2) {
                for (rv rv2 : a[i2]) {
                    if (rv2.H != n2) continue;
                    this.n = n2;
                    return;
                }
            }
        }
    }

    @Override
    public ft m() {
        dh dh2 = new dh();
        this.b(dh2);
        return new gf(this.c, this.d, this.e, 3, dh2);
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        this.m = dh2.f("Primary");
        this.n = dh2.f("Secondary");
        this.l = dh2.f("Levels");
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("Primary", this.m);
        dh2.a("Secondary", this.n);
        dh2.a("Levels", this.l);
    }

    @Override
    public int a() {
        return 1;
    }

    @Override
    public add a(int n2) {
        if (n2 == 0) {
            return this.o;
        }
        return null;
    }

    @Override
    public add a(int n2, int n3) {
        if (n2 == 0 && this.o != null) {
            if (n3 >= this.o.b) {
                add add2 = this.o;
                this.o = null;
                return add2;
            }
            this.o.b -= n3;
            return new add(this.o.b(), n3, this.o.k());
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (n2 == 0 && this.o != null) {
            add add2 = this.o;
            this.o = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        if (n2 == 0) {
            this.o = add2;
        }
    }

    @Override
    public String b() {
        return this.k_() ? this.p : "container.beacon";
    }

    @Override
    public boolean k_() {
        return this.p != null && this.p.length() > 0;
    }

    public void a(String string) {
        this.p = string;
    }

    @Override
    public int d() {
        return 1;
    }

    @Override
    public boolean a(yz yz2) {
        if (this.b.o(this.c, this.d, this.e) != this) {
            return false;
        }
        return !(yz2.e((double)this.c + 0.5, (double)this.d + 0.5, (double)this.e + 0.5) > 64.0);
    }

    @Override
    public void f() {
    }

    @Override
    public void l_() {
    }

    @Override
    public boolean b(int n2, add add2) {
        return add2.b() == ade.bC || add2.b() == ade.i || add2.b() == ade.k || add2.b() == ade.j;
    }
}

