/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class api
extends aor
implements aph {
    private add[] a = new add[5];
    private String i;
    private int j = -1;

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.a = new add[this.a()];
        if (dh2.b("CustomName", 8)) {
            this.i = dh2.j("CustomName");
        }
        this.j = dh2.f("TransferCooldown");
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            byte by2 = dh3.d("Slot");
            if (by2 < 0 || by2 >= this.a.length) continue;
            this.a[by2] = add.a(dh3);
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.a[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
        dh2.a("TransferCooldown", this.j);
        if (this.k_()) {
            dh2.a("CustomName", this.i);
        }
    }

    @Override
    public void e() {
        super.e();
    }

    @Override
    public int a() {
        return this.a.length;
    }

    @Override
    public add a(int n2) {
        return this.a[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.a[n2] != null) {
            if (this.a[n2].b <= n3) {
                add add2 = this.a[n2];
                this.a[n2] = null;
                return add2;
            }
            add add3 = this.a[n2].a(n3);
            if (this.a[n2].b == 0) {
                this.a[n2] = null;
            }
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.a[n2] != null) {
            add add2 = this.a[n2];
            this.a[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.a[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
    }

    @Override
    public String b() {
        return this.k_() ? this.i : "container.hopper";
    }

    @Override
    public boolean k_() {
        return this.i != null && this.i.length() > 0;
    }

    public void a(String string) {
        this.i = string;
    }

    @Override
    public int d() {
        return 64;
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
        return true;
    }

    @Override
    public void h() {
        if (this.b == null || this.b.E) {
            return;
        }
        --this.j;
        if (!this.j()) {
            this.c(0);
            this.i();
        }
    }

    public boolean i() {
        if (this.b == null || this.b.E) {
            return false;
        }
        if (!this.j() && aln.c(this.p())) {
            boolean bl2 = false;
            if (!this.k()) {
                bl2 = this.y();
            }
            if (!this.l()) {
                boolean bl3 = bl2 = api.a(this) || bl2;
            }
            if (bl2) {
                this.c(8);
                this.e();
                return true;
            }
        }
        return false;
    }

    private boolean k() {
        for (add add2 : this.a) {
            if (add2 == null) continue;
            return false;
        }
        return true;
    }

    private boolean l() {
        for (add add2 : this.a) {
            if (add2 != null && add2.b == add2.e()) continue;
            return false;
        }
        return true;
    }

    private boolean y() {
        rb rb2 = this.z();
        if (rb2 == null) {
            return false;
        }
        int n2 = q.a[aln.b(this.p())];
        if (this.a(rb2, n2)) {
            return false;
        }
        for (int i2 = 0; i2 < this.a(); ++i2) {
            if (this.a(i2) == null) continue;
            add add2 = this.a(i2).m();
            add add3 = api.a(rb2, this.a(i2, 1), n2);
            if (add3 == null || add3.b == 0) {
                rb2.e();
                return true;
            }
            this.a(i2, add2);
        }
        return false;
    }

    private boolean a(rb rb2, int n2) {
        if (rb2 instanceof rl && n2 > -1) {
            rl rl2 = (rl)rb2;
            int[] nArray = rl2.c(n2);
            for (int i2 = 0; i2 < nArray.length; ++i2) {
                add add2 = rl2.a(nArray[i2]);
                if (add2 != null && add2.b == add2.e()) continue;
                return false;
            }
        } else {
            int n3 = rb2.a();
            for (int i3 = 0; i3 < n3; ++i3) {
                add add3 = rb2.a(i3);
                if (add3 != null && add3.b == add3.e()) continue;
                return false;
            }
        }
        return true;
    }

    private static boolean b(rb rb2, int n2) {
        if (rb2 instanceof rl && n2 > -1) {
            rl rl2 = (rl)rb2;
            int[] nArray = rl2.c(n2);
            for (int i2 = 0; i2 < nArray.length; ++i2) {
                if (rl2.a(nArray[i2]) == null) continue;
                return false;
            }
        } else {
            int n3 = rb2.a();
            for (int i3 = 0; i3 < n3; ++i3) {
                if (rb2.a(i3) == null) continue;
                return false;
            }
        }
        return true;
    }

    public static boolean a(aph aph2) {
        rb rb2 = api.b(aph2);
        if (rb2 != null) {
            int n2 = 0;
            if (api.b(rb2, n2)) {
                return false;
            }
            if (rb2 instanceof rl && n2 > -1) {
                rl rl2 = (rl)rb2;
                int[] nArray = rl2.c(n2);
                for (int i2 = 0; i2 < nArray.length; ++i2) {
                    if (!api.a(aph2, rb2, nArray[i2], n2)) continue;
                    return true;
                }
            } else {
                int n3 = rb2.a();
                for (int i3 = 0; i3 < n3; ++i3) {
                    if (!api.a(aph2, rb2, i3, n2)) continue;
                    return true;
                }
            }
        } else {
            xk xk2 = api.a(aph2.w(), aph2.x(), aph2.aD() + 1.0, aph2.aE());
            if (xk2 != null) {
                return api.a(aph2, xk2);
            }
        }
        return false;
    }

    private static boolean a(aph aph2, rb rb2, int n2, int n3) {
        add add2 = rb2.a(n2);
        if (add2 != null && api.b(rb2, add2, n2, n3)) {
            add add3 = add2.m();
            add add4 = api.a(aph2, rb2.a(n2, 1), -1);
            if (add4 == null || add4.b == 0) {
                rb2.e();
                return true;
            }
            rb2.a(n2, add3);
        }
        return false;
    }

    public static boolean a(rb rb2, xk xk2) {
        boolean bl2 = false;
        if (xk2 == null) {
            return false;
        }
        add add2 = xk2.f().m();
        add add3 = api.a(rb2, add2, -1);
        if (add3 == null || add3.b == 0) {
            bl2 = true;
            xk2.B();
        } else {
            xk2.a(add3);
        }
        return bl2;
    }

    public static add a(rb rb2, add add2, int n2) {
        if (rb2 instanceof rl && n2 > -1) {
            rl rl2 = (rl)rb2;
            int[] nArray = rl2.c(n2);
            for (int i2 = 0; i2 < nArray.length && add2 != null && add2.b > 0; ++i2) {
                add2 = api.c(rb2, add2, nArray[i2], n2);
            }
        } else {
            int n3 = rb2.a();
            for (int i3 = 0; i3 < n3 && add2 != null && add2.b > 0; ++i3) {
                add2 = api.c(rb2, add2, i3, n2);
            }
        }
        if (add2 != null && add2.b == 0) {
            add2 = null;
        }
        return add2;
    }

    private static boolean a(rb rb2, add add2, int n2, int n3) {
        if (!rb2.b(n2, add2)) {
            return false;
        }
        return !(rb2 instanceof rl) || ((rl)rb2).a(n2, add2, n3);
    }

    private static boolean b(rb rb2, add add2, int n2, int n3) {
        return !(rb2 instanceof rl) || ((rl)rb2).b(n2, add2, n3);
    }

    private static add c(rb rb2, add add2, int n2, int n3) {
        add add3 = rb2.a(n2);
        if (api.a(rb2, add2, n2, n3)) {
            boolean bl2 = false;
            if (add3 == null) {
                rb2.a(n2, add2);
                add2 = null;
                bl2 = true;
            } else if (api.a(add3, add2)) {
                int n4 = add2.e() - add3.b;
                int n5 = Math.min(add2.b, n4);
                add2.b -= n5;
                add3.b += n5;
                boolean bl3 = bl2 = n5 > 0;
            }
            if (bl2) {
                if (rb2 instanceof api) {
                    ((api)rb2).c(8);
                    rb2.e();
                }
                rb2.e();
            }
        }
        return add2;
    }

    private rb z() {
        int n2 = aln.b(this.p());
        return api.b(this.w(), this.c + q.b[n2], (double)(this.d + q.c[n2]), (double)(this.e + q.d[n2]));
    }

    public static rb b(aph aph2) {
        return api.b(aph2.w(), aph2.x(), aph2.aD() + 1.0, aph2.aE());
    }

    public static xk a(ahb ahb2, double d2, double d3, double d4) {
        List list = ahb2.a(xk.class, azt.a(d2, d3, d4, d2 + 1.0, d3 + 1.0, d4 + 1.0), sj.a);
        if (list.size() > 0) {
            return (xk)list.get(0);
        }
        return null;
    }

    public static rb b(ahb ahb2, double d2, double d3, double d4) {
        Object object;
        int n2;
        int n3;
        rb rb2 = null;
        int n4 = qh.c(d2);
        aor aor2 = ahb2.o(n4, n3 = qh.c(d3), n2 = qh.c(d4));
        if (aor2 != null && aor2 instanceof rb && (rb2 = (rb)((Object)aor2)) instanceof aow && (object = ahb2.a(n4, n3, n2)) instanceof ajx) {
            rb2 = ((ajx)object).m(ahb2, n4, n3, n2);
        }
        if (rb2 == null && (object = ahb2.a((sa)null, azt.a(d2, d3, d4, d2 + 1.0, d3 + 1.0, d4 + 1.0), sj.c)) != null && object.size() > 0) {
            rb2 = (rb)object.get(ahb2.s.nextInt(object.size()));
        }
        return rb2;
    }

    private static boolean a(add add2, add add3) {
        if (add2.b() != add3.b()) {
            return false;
        }
        if (add2.k() != add3.k()) {
            return false;
        }
        if (add2.b > add2.e()) {
            return false;
        }
        return add.a(add2, add3);
    }

    @Override
    public double x() {
        return this.c;
    }

    @Override
    public double aD() {
        return this.d;
    }

    @Override
    public double aE() {
        return this.e;
    }

    public void c(int n2) {
        this.j = n2;
    }

    public boolean j() {
        return this.j > 0;
    }
}

