/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class va
extends ui {
    private yv a;
    private sv b;
    private double c;
    private int d;

    public va(yv yv2, double d2) {
        this.a = yv2;
        this.c = d2;
        this.a(1);
    }

    @Override
    public boolean a() {
        if (this.a.d() >= 0) {
            return false;
        }
        if (this.a.aI().nextInt(400) != 0) {
            return false;
        }
        List list = this.a.o.a(yv.class, this.a.C.b(6.0, 3.0, 6.0));
        double d2 = Double.MAX_VALUE;
        Object object = list.iterator();
        while (object.hasNext()) {
            double d3;
            yv yv2 = (yv)object.next();
            if (yv2 == this.a || yv2.cb() || yv2.d() >= 0 || (d3 = yv2.f(this.a)) > d2) continue;
            d2 = d3;
            this.b = yv2;
        }
        return this.b != null || (object = vx.a(this.a, 16, 3)) != null;
    }

    @Override
    public boolean b() {
        return this.d > 0;
    }

    @Override
    public void c() {
        if (this.b != null) {
            this.a.j(true);
        }
        this.d = 1000;
    }

    @Override
    public void d() {
        this.a.j(false);
        this.b = null;
    }

    @Override
    public void e() {
        --this.d;
        if (this.b != null) {
            if (this.a.f(this.b) > 4.0) {
                this.a.m().a(this.b, this.c);
            }
        } else if (this.a.m().g()) {
            azw azw2 = vx.a(this.a, 16, 3);
            if (azw2 == null) {
                return;
            }
            this.a.m().a(azw2.a, azw2.b, azw2.c, this.c);
        }
    }
}

