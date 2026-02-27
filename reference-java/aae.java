/*
 * Decompiled with CFR 0.152.
 */
public class aae
implements rb {
    private add[] a;
    private int b;
    private zs c;

    public aae(zs zs2, int n2, int n3) {
        int n4 = n2 * n3;
        this.a = new add[n4];
        this.c = zs2;
        this.b = n2;
    }

    @Override
    public int a() {
        return this.a.length;
    }

    @Override
    public add a(int n2) {
        if (n2 >= this.a()) {
            return null;
        }
        return this.a[n2];
    }

    public add b(int n2, int n3) {
        if (n2 < 0 || n2 >= this.b) {
            return null;
        }
        int n4 = n2 + n3 * this.b;
        return this.a(n4);
    }

    @Override
    public String b() {
        return "container.crafting";
    }

    @Override
    public boolean k_() {
        return false;
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
    public add a(int n2, int n3) {
        if (this.a[n2] != null) {
            if (this.a[n2].b <= n3) {
                add add2 = this.a[n2];
                this.a[n2] = null;
                this.c.a(this);
                return add2;
            }
            add add3 = this.a[n2].a(n3);
            if (this.a[n2].b == 0) {
                this.a[n2] = null;
            }
            this.c.a(this);
            return add3;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.a[n2] = add2;
        this.c.a(this);
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public void e() {
    }

    @Override
    public boolean a(yz yz2) {
        return true;
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
}

