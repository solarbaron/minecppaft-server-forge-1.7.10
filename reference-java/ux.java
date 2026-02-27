/*
 * Decompiled with CFR 0.152.
 */
public class ux
extends ui {
    private wt a;
    private yv b;
    private int c;

    public ux(wt wt2) {
        this.a = wt2;
        this.a(3);
    }

    @Override
    public boolean a() {
        if (!this.a.o.w()) {
            return false;
        }
        if (this.a.aI().nextInt(8000) != 0) {
            return false;
        }
        this.b = (yv)this.a.o.a(yv.class, this.a.C.b(6.0, 2.0, 6.0), (sa)this.a);
        return this.b != null;
    }

    @Override
    public boolean b() {
        return this.c > 0;
    }

    @Override
    public void c() {
        this.c = 400;
        this.a.a(true);
    }

    @Override
    public void d() {
        this.a.a(false);
        this.b = null;
    }

    @Override
    public void e() {
        this.a.j().a(this.b, 30.0f, 30.0f);
        --this.c;
    }
}

