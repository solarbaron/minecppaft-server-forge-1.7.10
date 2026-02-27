/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class uh
extends ui {
    wf a;
    wf b;
    double c;
    private int d;

    public uh(wf wf2, double d2) {
        this.a = wf2;
        this.c = d2;
    }

    @Override
    public boolean a() {
        if (this.a.d() >= 0) {
            return false;
        }
        List list = this.a.o.a(this.a.getClass(), this.a.C.b(8.0, 4.0, 8.0));
        wf wf2 = null;
        double d2 = Double.MAX_VALUE;
        for (wf wf3 : list) {
            double d3;
            if (wf3.d() < 0 || (d3 = this.a.f(wf3)) > d2) continue;
            d2 = d3;
            wf2 = wf3;
        }
        if (wf2 == null) {
            return false;
        }
        if (d2 < 9.0) {
            return false;
        }
        this.b = wf2;
        return true;
    }

    @Override
    public boolean b() {
        if (!this.b.Z()) {
            return false;
        }
        double d2 = this.a.f(this.b);
        return !(d2 < 9.0) && !(d2 > 256.0);
    }

    @Override
    public void c() {
        this.d = 0;
    }

    @Override
    public void d() {
        this.b = null;
    }

    @Override
    public void e() {
        if (--this.d > 0) {
            return;
        }
        this.d = 10;
        this.a.m().a(this.b, this.c);
    }
}

