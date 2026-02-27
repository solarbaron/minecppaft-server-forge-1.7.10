/*
 * Decompiled with CFR 0.152.
 */
public class xs
extends xl {
    public xs(ahb ahb2) {
        super(ahb2);
    }

    public xs(ahb ahb2, double d2, double d3, double d4) {
        super(ahb2, d2, d3, d4);
    }

    @Override
    public boolean c(yz yz2) {
        if (this.l != null && this.l instanceof yz && this.l != yz2) {
            return true;
        }
        if (this.l != null && this.l != yz2) {
            return false;
        }
        if (!this.o.E) {
            yz2.a(this);
        }
        return true;
    }

    @Override
    public int m() {
        return 0;
    }
}

