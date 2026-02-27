/*
 * Decompiled with CFR 0.152.
 */
public class aol
extends ajd {
    private final int a;

    protected aol(String string, awt awt2, int n2) {
        super(string, awt2);
        this.a = n2;
    }

    @Override
    protected int e(ahb ahb2, int n2, int n3, int n4) {
        int n5 = Math.min(ahb2.a(sa.class, this.a(n2, n3, n4)).size(), this.a);
        if (n5 <= 0) {
            return 0;
        }
        float f2 = (float)Math.min(this.a, n5) / (float)this.a;
        return qh.f(f2 * 15.0f);
    }

    @Override
    protected int c(int n2) {
        return n2;
    }

    @Override
    protected int d(int n2) {
        return n2;
    }

    @Override
    public int a(ahb ahb2) {
        return 10;
    }
}

