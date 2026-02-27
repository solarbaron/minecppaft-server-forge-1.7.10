/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class rh
implements rb {
    private String a;
    private int b;
    private add[] c;
    private List d;
    private boolean e;

    public rh(String string, boolean bl2, int n2) {
        this.a = string;
        this.e = bl2;
        this.b = n2;
        this.c = new add[n2];
    }

    public void a(rc rc2) {
        if (this.d == null) {
            this.d = new ArrayList();
        }
        this.d.add(rc2);
    }

    public void b(rc rc2) {
        this.d.remove(rc2);
    }

    @Override
    public add a(int n2) {
        if (n2 < 0 || n2 >= this.c.length) {
            return null;
        }
        return this.c[n2];
    }

    @Override
    public add a(int n2, int n3) {
        if (this.c[n2] != null) {
            if (this.c[n2].b <= n3) {
                add add2 = this.c[n2];
                this.c[n2] = null;
                this.e();
                return add2;
            }
            add add3 = this.c[n2].a(n3);
            if (this.c[n2].b == 0) {
                this.c[n2] = null;
            }
            this.e();
            return add3;
        }
        return null;
    }

    @Override
    public add a_(int n2) {
        if (this.c[n2] != null) {
            add add2 = this.c[n2];
            this.c[n2] = null;
            return add2;
        }
        return null;
    }

    @Override
    public void a(int n2, add add2) {
        this.c[n2] = add2;
        if (add2 != null && add2.b > this.d()) {
            add2.b = this.d();
        }
        this.e();
    }

    @Override
    public int a() {
        return this.b;
    }

    @Override
    public String b() {
        return this.a;
    }

    @Override
    public boolean k_() {
        return this.e;
    }

    public void a(String string) {
        this.e = true;
        this.a = string;
    }

    @Override
    public int d() {
        return 64;
    }

    @Override
    public void e() {
        if (this.d != null) {
            for (int i2 = 0; i2 < this.d.size(); ++i2) {
                ((rc)this.d.get(i2)).a(this);
            }
        }
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

