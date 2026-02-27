/*
 * Decompiled with CFR 0.152.
 */
public class aaw
implements rb {
    private add[] a = new add[1];

    @Override
    public int a() {
        return 1;
    }

    @Override
    public add a(int n2) {
        return this.a[0];
    }

    @Override
    public String b() {
        return "Result";
    }

    @Override
    public boolean k_() {
        return false;
    }

    @Override
    public add a(int n2, int n3) {
        if (this.a[0] != null) {
            add add2 = this.a[0];
            this.a[0] = null;
            return add2;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.a[0] != null) {
            add add2 = this.a[0];
            this.a[0] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.a[0] = add2;
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

