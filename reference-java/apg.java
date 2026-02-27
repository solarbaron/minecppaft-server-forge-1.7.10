/*
 * Decompiled with CFR 0.152.
 */
public class apg
extends aor
implements rl {
    private static final int[] k = new int[]{0};
    private static final int[] l = new int[]{2, 1};
    private static final int[] m = new int[]{1};
    private add[] n = new add[3];
    public int a;
    public int i;
    public int j;
    private String o;

    @Override
    public int a() {
        return this.n.length;
    }

    @Override
    public add a(int n2) {
        return this.n[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.n[n2] != null) {
            if (this.n[n2].b <= n3) {
                add add2 = this.n[n2];
                this.n[n2] = null;
                return add2;
            }
            add add3 = this.n[n2].a(n3);
            if (this.n[n2].b == 0) {
                this.n[n2] = null;
            }
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.n[n2] != null) {
            add add2 = this.n[n2];
            this.n[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.n[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
    }

    @Override
    public String b() {
        return this.k_() ? this.o : "container.furnace";
    }

    @Override
    public boolean k_() {
        return this.o != null && this.o.length() > 0;
    }

    public void a(String string) {
        this.o = string;
    }

    @Override
    public void a(dh dh2) {
        super.a(dh2);
        dq dq2 = dh2.c("Items", 10);
        this.n = new add[this.a()];
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            dh dh3 = dq2.b(i2);
            byte by2 = dh3.d("Slot");
            if (by2 < 0 || by2 >= this.n.length) continue;
            this.n[by2] = add.a(dh3);
        }
        this.a = dh2.e("BurnTime");
        this.j = dh2.e("CookTime");
        this.i = apg.a(this.n[1]);
        if (dh2.b("CustomName", 8)) {
            this.o = dh2.j("CustomName");
        }
    }

    @Override
    public void b(dh dh2) {
        super.b(dh2);
        dh2.a("BurnTime", (short)this.a);
        dh2.a("CookTime", (short)this.j);
        dq dq2 = new dq();
        for (int i2 = 0; i2 < this.n.length; ++i2) {
            if (this.n[i2] == null) continue;
            dh dh3 = new dh();
            dh3.a("Slot", (byte)i2);
            this.n[i2].b(dh3);
            dq2.a(dh3);
        }
        dh2.a("Items", dq2);
        if (this.k_()) {
            dh2.a("CustomName", this.o);
        }
    }

    @Override
    public int d() {
        return 64;
    }

    public boolean i() {
        return this.a > 0;
    }

    @Override
    public void h() {
        boolean bl2 = this.a > 0;
        boolean bl3 = false;
        if (this.a > 0) {
            --this.a;
        }
        if (!this.b.E) {
            if (this.a != 0 || this.n[1] != null && this.n[0] != null) {
                if (this.a == 0 && this.k()) {
                    this.i = this.a = apg.a(this.n[1]);
                    if (this.a > 0) {
                        bl3 = true;
                        if (this.n[1] != null) {
                            --this.n[1].b;
                            if (this.n[1].b == 0) {
                                adb adb2 = this.n[1].b().t();
                                add add2 = this.n[1] = adb2 != null ? new add(adb2) : null;
                            }
                        }
                    }
                }
                if (this.i() && this.k()) {
                    ++this.j;
                    if (this.j == 200) {
                        this.j = 0;
                        this.j();
                        bl3 = true;
                    }
                } else {
                    this.j = 0;
                }
            }
            if (bl2 != this.a > 0) {
                bl3 = true;
                ale.a(this.a > 0, this.b, this.c, this.d, this.e);
            }
        }
        if (bl3) {
            this.e();
        }
    }

    private boolean k() {
        if (this.n[0] == null) {
            return false;
        }
        add add2 = afa.a().a(this.n[0]);
        if (add2 == null) {
            return false;
        }
        if (this.n[2] == null) {
            return true;
        }
        if (!this.n[2].a(add2)) {
            return false;
        }
        if (this.n[2].b < this.d() && this.n[2].b < this.n[2].e()) {
            return true;
        }
        return this.n[2].b < add2.e();
    }

    public void j() {
        if (!this.k()) {
            return;
        }
        add add2 = afa.a().a(this.n[0]);
        if (this.n[2] == null) {
            this.n[2] = add2.m();
        } else if (this.n[2].b() == add2.b()) {
            ++this.n[2].b;
        }
        --this.n[0].b;
        if (this.n[0].b <= 0) {
            this.n[0] = null;
        }
    }

    public static int a(add add2) {
        if (add2 == null) {
            return 0;
        }
        adb adb2 = add2.b();
        if (adb2 instanceof abh && aji.a(adb2) != ajn.a) {
            aji aji2 = aji.a(adb2);
            if (aji2 == ajn.bx) {
                return 150;
            }
            if (aji2.o() == awt.d) {
                return 300;
            }
            if (aji2 == ajn.ci) {
                return 16000;
            }
        }
        if (adb2 instanceof acg && ((acg)adb2).j().equals("WOOD")) {
            return 200;
        }
        if (adb2 instanceof aeh && ((aeh)adb2).j().equals("WOOD")) {
            return 200;
        }
        if (adb2 instanceof ada && ((ada)adb2).i().equals("WOOD")) {
            return 200;
        }
        if (adb2 == ade.y) {
            return 100;
        }
        if (adb2 == ade.h) {
            return 1600;
        }
        if (adb2 == ade.at) {
            return 20000;
        }
        if (adb2 == adb.a(ajn.g)) {
            return 100;
        }
        if (adb2 == ade.bj) {
            return 2400;
        }
        return 0;
    }

    public static boolean b(add add2) {
        return apg.a(add2) > 0;
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
        if (n2 == 2) {
            return false;
        }
        if (n2 == 1) {
            return apg.b(add2);
        }
        return true;
    }

    @Override
    public int[] c(int n2) {
        if (n2 == 0) {
            return l;
        }
        if (n2 == 1) {
            return k;
        }
        return m;
    }

    @Override
    public boolean a(int n2, add add2, int n3) {
        return this.b(n2, add2);
    }

    @Override
    public boolean b(int n2, add add2, int n3) {
        return n3 != 0 || n2 != 1 || add2.b() == ade.ar;
    }
}

