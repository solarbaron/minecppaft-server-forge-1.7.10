/*
 * Decompiled with CFR 0.152.
 */
public class un
extends ui {
    private sw b;
    protected sa a;
    private float c;
    private int d;
    private float e;
    private Class f;

    public un(sw sw2, Class clazz, float f2) {
        this.b = sw2;
        this.f = clazz;
        this.c = f2;
        this.e = 0.02f;
        this.a(2);
    }

    public un(sw sw2, Class clazz, float f2, float f3) {
        this.b = sw2;
        this.f = clazz;
        this.c = f2;
        this.e = f3;
        this.a(2);
    }

    @Override
    public boolean a() {
        if (this.b.aI().nextFloat() >= this.e) {
            return false;
        }
        if (this.b.o() != null) {
            this.a = this.b.o();
        }
        this.a = this.f == yz.class ? this.b.o.a((sa)this.b, this.c) : this.b.o.a(this.f, this.b.C.b(this.c, 3.0, this.c), (sa)this.b);
        return this.a != null;
    }

    @Override
    public boolean b() {
        if (!this.a.Z()) {
            return false;
        }
        if (this.b.f(this.a) > (double)(this.c * this.c)) {
            return false;
        }
        return this.d > 0;
    }

    @Override
    public void c() {
        this.d = 40 + this.b.aI().nextInt(40);
    }

    @Override
    public void d() {
        this.a = null;
    }

    @Override
    public void e() {
        this.b.j().a(this.a.s, this.a.t + (double)this.a.g(), this.a.u, 10.0f, this.b.x());
        --this.d;
    }
}

