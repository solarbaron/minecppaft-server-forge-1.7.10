/*
 * Decompiled with CFR 0.152.
 */
public class acy
extends acx {
    public acy(int n2, float f2, boolean bl2) {
        super(n2, f2, bl2);
        this.a(true);
    }

    @Override
    public adq f(add add2) {
        if (add2.k() == 0) {
            return adq.c;
        }
        return adq.d;
    }

    @Override
    protected void c(add add2, ahb ahb2, yz yz2) {
        if (!ahb2.E) {
            yz2.c(new rw(rv.x.H, 2400, 0));
        }
        if (add2.k() > 0) {
            if (!ahb2.E) {
                yz2.c(new rw(rv.l.H, 600, 4));
                yz2.c(new rw(rv.m.H, 6000, 0));
                yz2.c(new rw(rv.n.H, 6000, 0));
            }
        } else {
            super.c(add2, ahb2, yz2);
        }
    }
}

