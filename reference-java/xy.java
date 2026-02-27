/*
 * Decompiled with CFR 0.152.
 */
public class xy
extends yn {
    public xy(ahb ahb2) {
        super(ahb2);
        this.a(0.7f, 0.5f);
    }

    @Override
    protected void aD() {
        super.aD();
        this.a(yj.a).a(12.0);
    }

    @Override
    public boolean n(sa sa2) {
        if (super.n(sa2)) {
            if (sa2 instanceof sv) {
                int n2 = 0;
                if (this.o.r == rd.c) {
                    n2 = 7;
                } else if (this.o.r == rd.d) {
                    n2 = 15;
                }
                if (n2 > 0) {
                    ((sv)sa2).c(new rw(rv.u.H, n2 * 20, 0));
                }
            }
            return true;
        }
        return false;
    }

    @Override
    public sy a(sy sy2) {
        return sy2;
    }
}

