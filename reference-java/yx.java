/*
 * Decompiled with CFR 0.152.
 */
public class yx
implements rb {
    public add[] a = new add[36];
    public add[] b = new add[4];
    public int c;
    public yz d;
    private add g;
    public boolean e;

    public yx(yz yz2) {
        this.d = yz2;
    }

    public add h() {
        if (this.c < 9 && this.c >= 0) {
            return this.a[this.c];
        }
        return null;
    }

    public static int i() {
        return 9;
    }

    private int c(adb adb2) {
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] == null || this.a[i2].b() != adb2) continue;
            return i2;
        }
        return -1;
    }

    private int d(add add2) {
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] == null || this.a[i2].b() != add2.b() || !this.a[i2].f() || this.a[i2].b >= this.a[i2].e() || this.a[i2].b >= this.d() || this.a[i2].h() && this.a[i2].k() != add2.k() || !add.a(this.a[i2], add2)) continue;
            return i2;
        }
        return -1;
    }

    public int j() {
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] != null) continue;
            return i2;
        }
        return -1;
    }

    public int a(adb adb2, int n2) {
        add add2;
        int n3;
        int n4 = 0;
        for (n3 = 0; n3 < this.a.length; ++n3) {
            add2 = this.a[n3];
            if (add2 == null || adb2 != null && add2.b() != adb2 || n2 > -1 && add2.k() != n2) continue;
            n4 += add2.b;
            this.a[n3] = null;
        }
        for (n3 = 0; n3 < this.b.length; ++n3) {
            add2 = this.b[n3];
            if (add2 == null || adb2 != null && add2.b() != adb2 || n2 > -1 && add2.k() != n2) continue;
            n4 += add2.b;
            this.b[n3] = null;
        }
        if (this.g != null) {
            if (adb2 != null && this.g.b() != adb2) {
                return n4;
            }
            if (n2 > -1 && this.g.k() != n2) {
                return n4;
            }
            n4 += this.g.b;
            this.b((add)null);
        }
        return n4;
    }

    private int e(add add2) {
        int n2;
        adb adb2 = add2.b();
        int n3 = add2.b;
        if (add2.e() == 1) {
            int n4 = this.j();
            if (n4 < 0) {
                return n3;
            }
            if (this.a[n4] == null) {
                this.a[n4] = add.b(add2);
            }
            return 0;
        }
        int n5 = this.d(add2);
        if (n5 < 0) {
            n5 = this.j();
        }
        if (n5 < 0) {
            return n3;
        }
        if (this.a[n5] == null) {
            this.a[n5] = new add(adb2, 0, add2.k());
            if (add2.p()) {
                this.a[n5].d((dh)add2.q().b());
            }
        }
        if ((n2 = n3) > this.a[n5].e() - this.a[n5].b) {
            n2 = this.a[n5].e() - this.a[n5].b;
        }
        if (n2 > this.d() - this.a[n5].b) {
            n2 = this.d() - this.a[n5].b;
        }
        if (n2 == 0) {
            return n3;
        }
        this.a[n5].b += n2;
        this.a[n5].c = 5;
        return n3 -= n2;
    }

    public void k() {
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2] == null) continue;
            this.a[i2].a(this.d.o, this.d, i2, this.c == i2);
        }
    }

    public boolean a(adb adb2) {
        int n2 = this.c(adb2);
        if (n2 < 0) {
            return false;
        }
        if (--this.a[n2].b <= 0) {
            this.a[n2] = null;
        }
        return true;
    }

    public boolean b(adb adb2) {
        int n2 = this.c(adb2);
        return n2 >= 0;
    }

    public boolean a(add add2) {
        if (add2 == null || add2.b == 0 || add2.b() == null) {
            return false;
        }
        try {
            if (!add2.i()) {
                int n2;
                do {
                    n2 = add2.b;
                    add2.b = this.e(add2);
                } while (add2.b > 0 && add2.b < n2);
                if (add2.b == n2 && this.d.bE.d) {
                    add2.b = 0;
                    return true;
                }
                return add2.b < n2;
            }
            int n3 = this.j();
            if (n3 >= 0) {
                this.a[n3] = add.b(add2);
                this.a[n3].c = 5;
                add2.b = 0;
                return true;
            }
            if (this.d.bE.d) {
                add2.b = 0;
                return true;
            }
            return false;
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Adding item to inventory");
            k k2 = b2.a("Item being added");
            k2.a("Item ID", adb.b(add2.b()));
            k2.a("Item data", add2.k());
            k2.a("Item name", new yy(this, add2));
            throw new s(b2);
        }
    }

    @Override
    public add a(int n2, int n3) {
        add[] addArray = this.a;
        if (n2 >= this.a.length) {
            addArray = this.b;
            n2 -= this.a.length;
        }
        if (addArray[n2] != null) {
            if (addArray[n2].b <= n3) {
                add add2 = addArray[n2];
                addArray[n2] = null;
                return add2;
            }
            add add3 = addArray[n2].a(n3);
            if (addArray[n2].b == 0) {
                addArray[n2] = null;
            }
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        add[] addArray = this.a;
        if (n2 >= this.a.length) {
            addArray = this.b;
            n2 -= this.a.length;
        }
        if (addArray[n2] != null) {
            add add2 = addArray[n2];
            addArray[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        add[] addArray = this.a;
        if (n2 >= addArray.length) {
            n2 -= addArray.length;
            addArray = this.b;
        }
        addArray[n2] = add2;
    }

    public float a(aji aji2) {
        float f2 = 1.0f;
        if (this.a[this.c] != null) {
            f2 *= this.a[this.c].a(aji2);
        }
        return f2;
    }

    public dq a(dq dq2) {
        dh dh2;
        int n2;
        for (n2 = 0; n2 < this.a.length; ++n2) {
            if (this.a[n2] == null) continue;
            dh2 = new dh();
            dh2.a("Slot", (byte)n2);
            this.a[n2].b(dh2);
            dq2.a(dh2);
        }
        for (n2 = 0; n2 < this.b.length; ++n2) {
            if (this.b[n2] == null) continue;
            dh2 = new dh();
            dh2.a("Slot", (byte)(n2 + 100));
            this.b[n2].b(dh2);
            dq2.a(dh2);
        }
        return dq2;
    }

    public void b(dq dq2) {
        this.a = new add[36];
        this.b = new add[4];
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh2 = dq2.b(i2);
            int n2 = dh2.d("Slot") & 0xFF;
            add add2 = add.a(dh2);
            if (add2 == null) continue;
            if (n2 >= 0 && n2 < this.a.length) {
                this.a[n2] = add2;
            }
            if (n2 < 100 || n2 >= this.b.length + 100) continue;
            this.b[n2 - 100] = add2;
        }
    }

    @Override
    public int a() {
        return this.a.length + 4;
    }

    @Override
    public add a(int n2) {
        add[] addArray = this.a;
        if (n2 >= addArray.length) {
            n2 -= addArray.length;
            addArray = this.b;
        }
        return addArray[n2];
    }

    @Override
    public String b() {
        return "container.inventory";
    }

    @Override
    public boolean k_() {
        return false;
    }

    @Override
    public int d() {
        return 64;
    }

    public boolean b(aji aji2) {
        if (aji2.o().l()) {
            return true;
        }
        add add2 = this.a(this.c);
        if (add2 != null) {
            return add2.b(aji2);
        }
        return false;
    }

    public add d(int n2) {
        return this.b[n2];
    }

    public int l() {
        int n2 = 0;
        for (int i2 = 0; i2 < this.b.length; ++i2) {
            if (this.b[i2] == null || !(this.b[i2].b() instanceof abb)) continue;
            int n3 = ((abb)this.b[i2].b()).c;
            n2 += n3;
        }
        return n2;
    }

    public void a(float f2) {
        if ((f2 /= 4.0f) < 1.0f) {
            f2 = 1.0f;
        }
        for (int i2 = 0; i2 < this.b.length; ++i2) {
            if (this.b[i2] == null || !(this.b[i2].b() instanceof abb)) continue;
            this.b[i2].a((int)f2, (sv)this.d);
            if (this.b[i2].b != 0) continue;
            this.b[i2] = null;
        }
    }

    public void m() {
        int n2;
        for (n2 = 0; n2 < this.a.length; ++n2) {
            if (this.a[n2] == null) continue;
            this.d.a(this.a[n2], true, false);
            this.a[n2] = null;
        }
        for (n2 = 0; n2 < this.b.length; ++n2) {
            if (this.b[n2] == null) continue;
            this.d.a(this.b[n2], true, false);
            this.b[n2] = null;
        }
    }

    @Override
    public void e() {
        this.e = true;
    }

    public void b(add add2) {
        this.g = add2;
    }

    public add o() {
        return this.g;
    }

    @Override
    public boolean a(yz yz2) {
        if (this.d.K) {
            return false;
        }
        return !(yz2.f(this.d) > 64.0);
    }

    public boolean c(add add2) {
        int n2;
        for (n2 = 0; n2 < this.b.length; ++n2) {
            if (this.b[n2] == null || !this.b[n2].a(add2)) continue;
            return true;
        }
        for (n2 = 0; n2 < this.a.length; ++n2) {
            if (this.a[n2] == null || !this.a[n2].a(add2)) continue;
            return true;
        }
        return false;
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

    public void b(yx yx2) {
        int n2;
        for (n2 = 0; n2 < this.a.length; ++n2) {
            this.a[n2] = add.b(yx2.a[n2]);
        }
        for (n2 = 0; n2 < this.b.length; ++n2) {
            this.b[n2] = add.b(yx2.b[n2]);
        }
        this.c = yx2.c;
    }
}

