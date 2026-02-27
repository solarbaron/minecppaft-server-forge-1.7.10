/*
 * Decompiled with CFR 0.152.
 */
public class ra
implements rb {
    private String a;
    private rb b;
    private rb c;

    public ra(String string, rb rb2, rb rb3) {
        this.a = string;
        if (rb2 == null) {
            rb2 = rb3;
        }
        if (rb3 == null) {
            rb3 = rb2;
        }
        this.b = rb2;
        this.c = rb3;
    }

    @Override
    public int a() {
        return this.b.a() + this.c.a();
    }

    public boolean a(rb rb2) {
        return this.b == rb2 || this.c == rb2;
    }

    @Override
    public String b() {
        if (this.b.k_()) {
            return this.b.b();
        }
        if (this.c.k_()) {
            return this.c.b();
        }
        return this.a;
    }

    @Override
    public boolean k_() {
        return this.b.k_() || this.c.k_();
    }

    @Override
    public add a(int n2) {
        if (n2 >= this.b.a()) {
            return this.c.a(n2 - this.b.a());
        }
        return this.b.a(n2);
    }

    @Override
    public add a(int n2, int n3) {
        if (n2 >= this.b.a()) {
            return this.c.a(n2 - this.b.a(), n3);
        }
        return this.b.a(n2, n3);
    }

    @Override
    public add a_(int n2) {
        if (n2 >= this.b.a()) {
            return this.c.a_(n2 - this.b.a());
        }
        return this.b.a_(n2);
    }

    @Override
    public void a(int n2, add add2) {
        if (n2 >= this.b.a()) {
            this.c.a(n2 - this.b.a(), add2);
        } else {
            this.b.a(n2, add2);
        }
    }

    @Override
    public int d() {
        return this.b.d();
    }

    @Override
    public void e() {
        this.b.e();
        this.c.e();
    }

    @Override
    public boolean a(yz yz2) {
        return this.b.a(yz2) && this.c.a(yz2);
    }

    @Override
    public void f() {
        this.b.f();
        this.c.f();
    }

    @Override
    public void l_() {
        this.b.l_();
        this.c.l_();
    }

    @Override
    public boolean b(int n2, add add2) {
        return true;
    }
}

