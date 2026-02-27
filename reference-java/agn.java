/*
 * Decompiled with CFR 0.152.
 */
public class agn {
    private add a;
    private add b;
    private add c;
    private int d;
    private int e;

    public agn(dh dh2) {
        this.a(dh2);
    }

    public agn(add add2, add add3, add add4) {
        this.a = add2;
        this.b = add3;
        this.c = add4;
        this.e = 7;
    }

    public agn(add add2, add add3) {
        this(add2, null, add3);
    }

    public agn(add add2, adb adb2) {
        this(add2, new add(adb2));
    }

    public add a() {
        return this.a;
    }

    public add b() {
        return this.b;
    }

    public boolean c() {
        return this.b != null;
    }

    public add d() {
        return this.c;
    }

    public boolean a(agn agn2) {
        if (this.a.b() != agn2.a.b() || this.c.b() != agn2.c.b()) {
            return false;
        }
        return this.b == null && agn2.b == null || this.b != null && agn2.b != null && this.b.b() == agn2.b.b();
    }

    public boolean b(agn agn2) {
        return this.a(agn2) && (this.a.b < agn2.a.b || this.b != null && this.b.b < agn2.b.b);
    }

    public void f() {
        ++this.d;
    }

    public void a(int n2) {
        this.e += n2;
    }

    public boolean g() {
        return this.d >= this.e;
    }

    public void a(dh dh2) {
        dh dh3 = dh2.m("buy");
        this.a = add.a(dh3);
        dh dh4 = dh2.m("sell");
        this.c = add.a(dh4);
        if (dh2.b("buyB", 10)) {
            this.b = add.a(dh2.m("buyB"));
        }
        if (dh2.b("uses", 99)) {
            this.d = dh2.f("uses");
        }
        this.e = dh2.b("maxUses", 99) ? dh2.f("maxUses") : 7;
    }

    public dh i() {
        dh dh2 = new dh();
        dh2.a("buy", this.a.b(new dh()));
        dh2.a("sell", this.c.b(new dh()));
        if (this.b != null) {
            dh2.a("buyB", this.b.b(new dh()));
        }
        dh2.a("uses", this.d);
        dh2.a("maxUses", this.e);
        return dh2;
    }
}

