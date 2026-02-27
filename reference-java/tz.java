/*
 * Decompiled with CFR 0.152.
 */
public class tz
extends uc {
    private int i;
    private int j = -1;

    public tz(sw sw2) {
        super(sw2);
    }

    @Override
    public boolean a() {
        if (!super.a()) {
            return false;
        }
        if (!this.a.o.O().b("mobGriefing")) {
            return false;
        }
        return !this.e.f((ahl)this.a.o, this.b, this.c, this.d);
    }

    @Override
    public void c() {
        super.c();
        this.i = 0;
    }

    @Override
    public boolean b() {
        double d2 = this.a.e(this.b, this.c, this.d);
        return this.i <= 240 && !this.e.f((ahl)this.a.o, this.b, this.c, this.d) && d2 < 4.0;
    }

    @Override
    public void d() {
        super.d();
        this.a.o.d(this.a.y(), this.b, this.c, this.d, -1);
    }

    @Override
    public void e() {
        super.e();
        if (this.a.aI().nextInt(20) == 0) {
            this.a.o.c(1010, this.b, this.c, this.d, 0);
        }
        ++this.i;
        int n2 = (int)((float)this.i / 240.0f * 10.0f);
        if (n2 != this.j) {
            this.a.o.d(this.a.y(), this.b, this.c, this.d, n2);
            this.j = n2;
        }
        if (this.i == 240 && this.a.o.r == rd.d) {
            this.a.o.f(this.b, this.c, this.d);
            this.a.o.c(1012, this.b, this.c, this.d, 0);
            this.a.o.c(2001, this.b, this.c, this.d, aji.b(this.e));
        }
    }
}

