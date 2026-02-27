/*
 * Decompiled with CFR 0.152.
 */
public class tw
extends ui {
    public final sj a = new tx(this);
    private td b;
    private double c;
    private double d;
    private sa e;
    private float f;
    private ayf g;
    private vv h;
    private Class i;

    public tw(td td2, Class clazz, float f2, double d2, double d3) {
        this.b = td2;
        this.i = clazz;
        this.f = f2;
        this.c = d2;
        this.d = d3;
        this.h = td2.m();
        this.a(1);
    }

    @Override
    public boolean a() {
        Object object;
        if (this.i == yz.class) {
            if (this.b instanceof tg && ((tg)this.b).bZ()) {
                return false;
            }
            this.e = this.b.o.a((sa)this.b, this.f);
            if (this.e == null) {
                return false;
            }
        } else {
            object = this.b.o.a(this.i, this.b.C.b(this.f, 3.0, this.f), this.a);
            if (object.isEmpty()) {
                return false;
            }
            this.e = (sa)object.get(0);
        }
        if ((object = vx.b(this.b, 16, 7, azw.a(this.e.s, this.e.t, this.e.u))) == null) {
            return false;
        }
        if (this.e.e(((azw)object).a, ((azw)object).b, ((azw)object).c) < this.e.f(this.b)) {
            return false;
        }
        this.g = this.h.a(((azw)object).a, ((azw)object).b, ((azw)object).c);
        if (this.g == null) {
            return false;
        }
        return this.g.b((azw)object);
    }

    @Override
    public boolean b() {
        return !this.h.g();
    }

    @Override
    public void c() {
        this.h.a(this.g, this.c);
    }

    @Override
    public void d() {
        this.e = null;
    }

    @Override
    public void e() {
        if (this.b.f(this.e) < 49.0) {
            this.b.m().a(this.d);
        } else {
            this.b.m().a(this.c);
        }
    }

    static /* synthetic */ td a(tw tw2) {
        return tw2.b;
    }
}

