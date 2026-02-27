/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ue
extends ui {
    private td a;
    private double b;
    private double c;
    private double d;
    private double e;
    private ahb f;

    public ue(td td2, double d2) {
        this.a = td2;
        this.e = d2;
        this.f = td2.o;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (!this.f.w()) {
            return false;
        }
        if (!this.a.al()) {
            return false;
        }
        if (!this.f.i(qh.c(this.a.s), (int)this.a.C.b, qh.c(this.a.u))) {
            return false;
        }
        azw azw2 = this.f();
        if (azw2 == null) {
            return false;
        }
        this.b = azw2.a;
        this.c = azw2.b;
        this.d = azw2.c;
        return true;
    }

    @Override
    public boolean b() {
        return !this.a.m().g();
    }

    @Override
    public void c() {
        this.a.m().a(this.b, this.c, this.d, this.e);
    }

    private azw f() {
        Random random = this.a.aI();
        for (int i2 = 0; i2 < 10; ++i2) {
            int n2;
            int n3;
            int n4 = qh.c(this.a.s + (double)random.nextInt(20) - 10.0);
            if (this.f.i(n4, n3 = qh.c(this.a.C.b + (double)random.nextInt(6) - 3.0), n2 = qh.c(this.a.u + (double)random.nextInt(20) - 10.0)) || !(this.a.a(n4, n3, n2) < 0.0f)) continue;
            return azw.a(n4, n3, n2);
        }
        return null;
    }
}

