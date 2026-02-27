/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class us
extends ui {
    private td a;
    private double b;
    private ayf c;
    private vy d;
    private boolean e;
    private List f = new ArrayList();

    public us(td td2, double d2, boolean bl2) {
        this.a = td2;
        this.b = d2;
        this.e = bl2;
        this.a(1);
    }

    @Override
    public boolean a() {
        this.f();
        if (this.e && this.a.o.w()) {
            return false;
        }
        vz vz2 = this.a.o.A.a(qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u), 0);
        if (vz2 == null) {
            return false;
        }
        this.d = this.a(vz2);
        if (this.d == null) {
            return false;
        }
        boolean bl2 = this.a.m().c();
        this.a.m().b(false);
        this.c = this.a.m().a(this.d.a, this.d.b, this.d.c);
        this.a.m().b(bl2);
        if (this.c != null) {
            return true;
        }
        azw azw2 = vx.a(this.a, 10, 7, azw.a(this.d.a, this.d.b, this.d.c));
        if (azw2 == null) {
            return false;
        }
        this.a.m().b(false);
        this.c = this.a.m().a(azw2.a, azw2.b, azw2.c);
        this.a.m().b(bl2);
        return this.c != null;
    }

    @Override
    public boolean b() {
        if (this.a.m().g()) {
            return false;
        }
        float f2 = this.a.M + 4.0f;
        return this.a.e(this.d.a, this.d.b, this.d.c) > (double)(f2 * f2);
    }

    @Override
    public void c() {
        this.a.m().a(this.c, this.b);
    }

    @Override
    public void d() {
        if (this.a.m().g() || this.a.e(this.d.a, this.d.b, this.d.c) < 16.0) {
            this.f.add(this.d);
        }
    }

    private vy a(vz vz2) {
        vy vy2 = null;
        int n2 = Integer.MAX_VALUE;
        List list = vz2.f();
        for (vy vy3 : list) {
            int n3 = vy3.b(qh.c(this.a.s), qh.c(this.a.t), qh.c(this.a.u));
            if (n3 >= n2 || this.a(vy3)) continue;
            vy2 = vy3;
            n2 = n3;
        }
        return vy2;
    }

    private boolean a(vy vy2) {
        for (vy vy3 : this.f) {
            if (vy2.a != vy3.a || vy2.b != vy3.b || vy2.c != vy3.c) continue;
            return true;
        }
        return false;
    }

    private void f() {
        if (this.f.size() > 15) {
            this.f.remove(0);
        }
    }
}

