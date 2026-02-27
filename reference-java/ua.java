/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ua
extends ui {
    private wf d;
    ahb a;
    private wf e;
    int b;
    double c;

    public ua(wf wf2, double d2) {
        this.d = wf2;
        this.a = wf2.o;
        this.c = d2;
        this.a(3);
    }

    @Override
    public boolean a() {
        if (!this.d.ce()) {
            return false;
        }
        this.e = this.f();
        return this.e != null;
    }

    @Override
    public boolean b() {
        return this.e.Z() && this.e.ce() && this.b < 60;
    }

    @Override
    public void d() {
        this.e = null;
        this.b = 0;
    }

    @Override
    public void e() {
        this.d.j().a(this.e, 10.0f, (float)this.d.x());
        this.d.m().a(this.e, this.c);
        ++this.b;
        if (this.b >= 60 && this.d.f(this.e) < 9.0) {
            this.g();
        }
    }

    private wf f() {
        float f2 = 8.0f;
        List list = this.a.a(this.d.getClass(), this.d.C.b(f2, f2, f2));
        double d2 = Double.MAX_VALUE;
        wf wf2 = null;
        for (wf wf3 : list) {
            if (!this.d.a(wf3) || !(this.d.f(wf3) < d2)) continue;
            wf2 = wf3;
            d2 = this.d.f(wf3);
        }
        return wf2;
    }

    private void g() {
        rx rx2 = this.d.a((rx)this.e);
        if (rx2 == null) {
            return;
        }
        yz yz2 = this.d.cd();
        if (yz2 == null && this.e.cd() != null) {
            yz2 = this.e.cd();
        }
        if (yz2 != null) {
            yz2.a(pp.x);
            if (this.d instanceof wh) {
                yz2.a(pc.H);
            }
        }
        this.d.c(6000);
        this.e.c(6000);
        this.d.cf();
        this.e.cf();
        rx2.c(-24000);
        rx2.b(this.d.s, this.d.t, this.d.u, 0.0f, 0.0f);
        this.a.d(rx2);
        Random random = this.d.aI();
        for (int i2 = 0; i2 < 7; ++i2) {
            double d2 = random.nextGaussian() * 0.02;
            double d3 = random.nextGaussian() * 0.02;
            double d4 = random.nextGaussian() * 0.02;
            this.a.a("heart", this.d.s + (double)(random.nextFloat() * this.d.M * 2.0f) - (double)this.d.M, this.d.t + 0.5 + (double)(random.nextFloat() * this.d.N), this.d.u + (double)(random.nextFloat() * this.d.M * 2.0f) - (double)this.d.M, d2, d3, d4);
        }
        if (this.a.O().b("doMobLoot")) {
            this.a.d(new sq(this.a, this.d.s, this.d.t, this.d.u, random.nextInt(7) + 1));
        }
    }
}

