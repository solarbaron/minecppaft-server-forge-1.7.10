/*
 * Decompiled with CFR 0.152.
 */
public class sp
implements sj {
    private final add d;

    public sp(add add2) {
        this.d = add2;
    }

    @Override
    public boolean a(sa sa2) {
        if (!sa2.Z()) {
            return false;
        }
        if (!(sa2 instanceof sv)) {
            return false;
        }
        sv sv2 = (sv)sa2;
        if (sv2.q(sw.b(this.d)) != null) {
            return false;
        }
        if (sv2 instanceof sw) {
            return ((sw)sv2).bJ();
        }
        return sv2 instanceof yz;
    }
}

