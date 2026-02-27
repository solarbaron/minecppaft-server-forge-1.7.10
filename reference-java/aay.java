/*
 * Decompiled with CFR 0.152.
 */
public class aay {
    private final int a;
    public final rb f;
    public int g;
    public int h;
    public int i;

    public aay(rb rb2, int n2, int n3, int n4) {
        this.f = rb2;
        this.a = n2;
        this.h = n3;
        this.i = n4;
    }

    public void a(add add2, add add3) {
        if (add2 == null || add3 == null) {
            return;
        }
        if (add2.b() != add3.b()) {
            return;
        }
        int n2 = add3.b - add2.b;
        if (n2 > 0) {
            this.a(add2, n2);
        }
    }

    protected void a(add add2, int n2) {
    }

    protected void b(add add2) {
    }

    public void a(yz yz2, add add2) {
        this.f();
    }

    public boolean a(add add2) {
        return true;
    }

    public add d() {
        return this.f.a(this.a);
    }

    public boolean e() {
        return this.d() != null;
    }

    public void c(add add2) {
        this.f.a(this.a, add2);
        this.f();
    }

    public void f() {
        this.f.e();
    }

    public int a() {
        return this.f.d();
    }

    public add a(int n2) {
        return this.f.a(this.a, n2);
    }

    public boolean a(rb rb2, int n2) {
        return rb2 == this.f && n2 == this.a;
    }

    public boolean a(yz yz2) {
        return true;
    }
}

