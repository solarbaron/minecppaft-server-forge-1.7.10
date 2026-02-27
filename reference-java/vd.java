/*
 * Decompiled with CFR 0.152.
 */
public class vd
extends ui {
    private final sw a;
    private final yi b;
    private sv c;
    private int d = -1;
    private double e;
    private int f;
    private int g;
    private int h;
    private float i;
    private float j;

    public vd(yi yi2, double d2, int n2, float f2) {
        this(yi2, d2, n2, n2, f2);
    }

    public vd(yi yi2, double d2, int n2, int n3, float f2) {
        if (!(yi2 instanceof sv)) {
            throw new IllegalArgumentException("ArrowAttackGoal requires Mob implements RangedAttackMob");
        }
        this.b = yi2;
        this.a = (sw)((Object)yi2);
        this.e = d2;
        this.g = n2;
        this.h = n3;
        this.i = f2;
        this.j = f2 * f2;
        this.a(3);
    }

    @Override
    public boolean a() {
        sv sv2 = this.a.o();
        if (sv2 == null) {
            return false;
        }
        this.c = sv2;
        return true;
    }

    @Override
    public boolean b() {
        return this.a() || !this.a.m().g();
    }

    @Override
    public void d() {
        this.c = null;
        this.f = 0;
        this.d = -1;
    }

    @Override
    public void e() {
        double d2 = this.a.e(this.c.s, this.c.C.b, this.c.u);
        boolean bl2 = this.a.n().a(this.c);
        this.f = bl2 ? ++this.f : 0;
        if (d2 > (double)this.j || this.f < 20) {
            this.a.m().a(this.c, this.e);
        } else {
            this.a.m().h();
        }
        this.a.j().a(this.c, 30.0f, 30.0f);
        if (--this.d == 0) {
            if (d2 > (double)this.j || !bl2) {
                return;
            }
            float f2 = qh.a(d2) / this.i;
            float f3 = f2;
            if (f3 < 0.1f) {
                f3 = 0.1f;
            }
            if (f3 > 1.0f) {
                f3 = 1.0f;
            }
            this.b.a(this.c, f3);
            this.d = qh.d(f2 * (float)(this.h - this.g) + (float)this.g);
        } else if (this.d < 0) {
            float f4 = qh.a(d2) / this.i;
            this.d = qh.d(f4 * (float)(this.h - this.g) + (float)this.g);
        }
    }
}

