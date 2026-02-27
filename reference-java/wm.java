/*
 * Decompiled with CFR 0.152.
 */
public class wm
extends wh {
    public wm(ahb ahb2) {
        super(ahb2);
        this.a(0.9f, 1.3f);
    }

    @Override
    public boolean a(yz yz2) {
        add add2 = yz2.bm.h();
        if (add2 != null && add2.b() == ade.z && this.d() >= 0) {
            if (add2.b == 1) {
                yz2.bm.a(yz2.bm.c, new add(ade.A));
                return true;
            }
            if (yz2.bm.a(new add(ade.A)) && !yz2.bE.d) {
                yz2.bm.a(yz2.bm.c, 1);
                return true;
            }
        }
        if (add2 != null && add2.b() == ade.aZ && this.d() >= 0) {
            this.B();
            this.o.a("largeexplode", this.s, this.t + (double)(this.N / 2.0f), this.u, 0.0, 0.0, 0.0);
            if (!this.o.E) {
                wh wh2 = new wh(this.o);
                wh2.b(this.s, this.t, this.u, this.y, this.z);
                wh2.g(this.aS());
                wh2.aM = this.aM;
                this.o.d(wh2);
                for (int i2 = 0; i2 < 5; ++i2) {
                    this.o.d(new xk(this.o, this.s, this.t + (double)this.N, this.u, new add(ajn.Q)));
                }
                add2.a(1, (sv)yz2);
                this.a("mob.sheep.shear", 1.0f, 1.0f);
            }
            return true;
        }
        return super.a(yz2);
    }

    public wm c(rx rx2) {
        return new wm(this.o);
    }

    @Override
    public /* synthetic */ wh b(rx rx2) {
        return this.c(rx2);
    }

    @Override
    public /* synthetic */ rx a(rx rx2) {
        return this.c(rx2);
    }
}

