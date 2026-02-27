/*
 * Decompiled with CFR 0.152.
 */
public class ud
extends ui {
    private sw b;
    private ahb c;
    int a;

    public ud(sw sw2) {
        this.b = sw2;
        this.c = sw2.o;
        this.a(7);
    }

    @Override
    public boolean a() {
        int n2;
        int n3;
        if (this.b.aI().nextInt(this.b.f() ? 50 : 1000) != 0) {
            return false;
        }
        int n4 = qh.c(this.b.s);
        if (this.c.a(n4, n3 = qh.c(this.b.t), n2 = qh.c(this.b.u)) == ajn.H && this.c.e(n4, n3, n2) == 1) {
            return true;
        }
        return this.c.a(n4, n3 - 1, n2) == ajn.c;
    }

    @Override
    public void c() {
        this.a = 40;
        this.c.a((sa)this.b, (byte)10);
        this.b.m().h();
    }

    @Override
    public void d() {
        this.a = 0;
    }

    @Override
    public boolean b() {
        return this.a > 0;
    }

    public int f() {
        return this.a;
    }

    @Override
    public void e() {
        int n2;
        int n3;
        this.a = Math.max(0, this.a - 1);
        if (this.a != 4) {
            return;
        }
        int n4 = qh.c(this.b.s);
        if (this.c.a(n4, n3 = qh.c(this.b.t), n2 = qh.c(this.b.u)) == ajn.H) {
            if (this.c.O().b("mobGriefing")) {
                this.c.a(n4, n3, n2, false);
            }
            this.b.p();
        } else if (this.c.a(n4, n3 - 1, n2) == ajn.c) {
            if (this.c.O().b("mobGriefing")) {
                this.c.c(2001, n4, n3 - 1, n2, aji.b(ajn.c));
                this.c.d(n4, n3 - 1, n2, ajn.d, 0, 2);
            }
            this.b.p();
        }
    }
}

