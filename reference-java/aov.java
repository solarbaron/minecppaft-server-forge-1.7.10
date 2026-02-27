/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class aov
extends aor
implements rl {
    private static final int[] a = new int[]{3};
    private static final int[] i = new int[]{0, 1, 2};
    private add[] j = new add[4];
    private int k;
    private int l;
    private adb m;
    private String n;

    @Override
    public String b() {
        return this.k_() ? this.n : "container.brewing";
    }

    @Override
    public boolean k_() {
        return this.n != null && this.n.length() > 0;
    }

    public void a(String string) {
        this.n = string;
    }

    @Override
    public int a() {
        return this.j.length;
    }

    @Override
    public void h() {
        if (this.k > 0) {
            --this.k;
            if (this.k == 0) {
                this.l();
                this.e();
            } else if (!this.k()) {
                this.k = 0;
                this.e();
            } else if (this.m != this.j[3].b()) {
                this.k = 0;
                this.e();
            }
        } else if (this.k()) {
            this.k = 400;
            this.m = this.j[3].b();
        }
        int n2 = this.j();
        if (n2 != this.l) {
            this.l = n2;
            this.b.a(this.c, this.d, this.e, n2, 2);
        }
        super.h();
    }

    public int i() {
        return this.k;
    }

    private boolean k() {
        if (this.j[3] == null || this.j[3].b <= 0) {
            return false;
        }
        add add2 = this.j[3];
        if (!add2.b().m(add2)) {
            return false;
        }
        boolean bl2 = false;
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.j[i2] == null || this.j[i2].b() != ade.bn) continue;
            int n2 = this.j[i2].k();
            int n3 = this.c(n2, add2);
            if (!adp.g(n2) && adp.g(n3)) {
                bl2 = true;
                break;
            }
            List list = ade.bn.c(n2);
            List list2 = ade.bn.c(n3);
            if (n2 > 0 && list == list2 || list != null && (list.equals(list2) || list2 == null) || n2 == n3) continue;
            bl2 = true;
            break;
        }
        return bl2;
    }

    private void l() {
        if (!this.k()) {
            return;
        }
        add add2 = this.j[3];
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.j[i2] == null || this.j[i2].b() != ade.bn) continue;
            int n2 = this.j[i2].k();
            int n3 = this.c(n2, add2);
            List list = ade.bn.c(n2);
            List list2 = ade.bn.c(n3);
            if (n2 > 0 && list == list2 || list != null && (list.equals(list2) || list2 == null)) {
                if (adp.g(n2) || !adp.g(n3)) continue;
                this.j[i2].b(n3);
                continue;
            }
            if (n2 == n3) continue;
            this.j[i2].b(n3);
        }
        if (add2.b().u()) {
            this.j[3] = new add(add2.b().t());
        } else {
            --this.j[3].b;
            if (this.j[3].b <= 0) {
                this.j[3] = null;
            }
        }
    }

    private int c(int n2, add add2) {
        if (add2 == null) {
            return n2;
        }
        if (add2.b().m(add2)) {
            return aen.a(n2, add2.b().i(add2));
        }
        return n2;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.j = new add[this.a()];
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            byte by2 = dh3.d("Slot");
            if (by2 < 0 || by2 >= this.j.length) continue;
            this.j[by2] = add.a(dh3);
        }
        this.k = dh2.e("BrewTime");
        if (dh2.b("CustomName", 8)) {
            this.n = dh2.j("CustomName");
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("BrewTime", (short)this.k);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.j.length; ++i2) {
            if (this.j[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.j[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
        if (this.k_()) {
            dh2.a("CustomName", this.n);
        }
    }

    @Override
    public add a(int n2) {
        if (n2 >= 0 && n2 < this.j.length) {
            return this.j[n2];
        }
        return null;
    }

    @Override
    public add a(int n2, int n3) {
        if (n2 >= 0 && n2 < this.j.length) {
            add add2 = this.j[n2];
            this.j[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (n2 >= 0 && n2 < this.j.length) {
            add add2 = this.j[n2];
            this.j[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        if (n2 >= 0 && n2 < this.j.length) {
            this.j[n2] = add2;
        }
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
        if (n2 == 3) {
            return add2.b().m(add2);
        }
        return add2.b() == ade.bn || add2.b() == ade.bo;
    }

    public int j() {
        int n2 = 0;
        for (int i2 = 0; i2 < 3; ++i2) {
            if (this.j[i2] == null) continue;
            n2 |= 1 << i2;
        }
        return n2;
    }

    @Override
    public int[] c(int n2) {
        if (n2 == 1) {
            return a;
        }
        return i;
    }

    @Override
    public boolean a(int n2, add add2, int n3) {
        return this.b(n2, add2);
    }

    @Override
    public boolean b(int n2, add add2, int n3) {
        return true;
    }
}

