/*
 * Decompiled with CFR 0.152.
 */
class ey {
    private final long a;
    private final int b;
    private final double c;

    private ey(long l2, int n2, double d2) {
        this.a = l2;
        this.b = n2;
        this.c = d2;
    }

    public ey a(long l2) {
        return new ey(l2 + this.a, this.b + 1, (l2 + this.a) / (long)(this.b + 1));
    }

    public long a() {
        return this.a;
    }

    public int b() {
        return this.b;
    }

    public String toString() {
        return "{totalBytes=" + this.a + ", count=" + this.b + ", averageBytes=" + this.c + '}';
    }

    /* synthetic */ ey(long l2, int n2, double d2, ev ev2) {
        this(l2, n2, d2);
    }

    static /* synthetic */ long a(ey ey2) {
        return ey2.a;
    }

    static /* synthetic */ int b(ey ey2) {
        return ey2.b;
    }
}

