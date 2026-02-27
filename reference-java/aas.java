/*
 * Decompiled with CFR 0.152.
 */
public class aas
implements rb {
    private final agm a;
    private add[] b = new add[3];
    private final yz c;
    private agn d;
    private int e;

    public aas(yz yz2, agm agm2) {
        this.c = yz2;
        this.a = agm2;
    }

    @Override
    public int a() {
        return this.b.length;
    }

    @Override
    public add a(int n2) {
        return this.b[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.b[n2] != null) {
            if (n2 == 2) {
                add add2 = this.b[n2];
                this.b[n2] = null;
                return add2;
            }
            if (this.b[n2].b <= n3) {
                add add3 = this.b[n2];
                this.b[n2] = null;
                if (this.d(n2)) {
                    this.h();
                }
                return add3;
            }
            add add4 = this.b[n2].a(n3);
            if (this.b[n2].b == 0) {
                this.b[n2] = null;
            }
            if (this.d(n2)) {
                this.h();
            }
            return add4;
        }
        return null;
    }

    private boolean d(int n2) {
        return n2 == 0 || n2 == 1;
    }

    @Override
    public add a_(int n2) {
        if (this.b[n2] != null) {
            add add2 = this.b[n2];
            this.b[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.b[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
        if (this.d(n2)) {
            this.h();
        }
    }

    @Override
    public String b() {
        return "mob.villager";
    }

    @Override
    public boolean k_() {
        return false;
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public boolean a(yz yz2) {
        return this.a.b() == yz2;
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
    public void e() {
        this.h();
    }

    public void h() {
        this.d = null;
        add add2 = this.b[0];
        add add3 = this.b[1];
        if (add2 == null) {
            add2 = add3;
            add3 = null;
        }
        if (add2 == null) {
            this.a(2, null);
        } else {
            ago ago2 = this.a.b(this.c);
            if (ago2 != null) {
                agn agn2 = ago2.a(add2, add3, this.e);
                if (agn2 != null && !agn2.g()) {
                    this.d = agn2;
                    this.a(2, agn2.d().m());
                } else if (add3 != null) {
                    agn2 = ago2.a(add3, add2, this.e);
                    if (agn2 != null && !agn2.g()) {
                        this.d = agn2;
                        this.a(2, agn2.d().m());
                    } else {
                        this.a(2, null);
                    }
                } else {
                    this.a(2, null);
                }
            }
        }
        this.a.a_(this.a(2));
    }

    public agn i() {
        return this.d;
    }

    public void c(int n2) {
        this.e = n2;
        this.h();
    }
}

