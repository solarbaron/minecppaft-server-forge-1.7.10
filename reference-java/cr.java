/*
 * Decompiled with CFR 0.152.
 */
public enum cr {
    a(0, 1, 0, -1, 0),
    b(1, 0, 0, 1, 0),
    c(2, 3, 0, 0, -1),
    d(3, 2, 0, 0, 1),
    e(4, 5, -1, 0, 0),
    f(5, 4, 1, 0, 0);

    private final int g;
    private final int h;
    private final int i;
    private final int j;
    private final int k;
    private static final cr[] l;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private cr(int n3, int n4, int n5) {
        void var7_5;
        void var6_4;
        this.g = n3;
        this.h = n4;
        this.i = n5;
        this.j = var6_4;
        this.k = var7_5;
    }

    public int c() {
        return this.i;
    }

    public int d() {
        return this.j;
    }

    public int e() {
        return this.k;
    }

    public static cr a(int n2) {
        return l[n2 % l.length];
    }

    static {
        l = new cr[6];
        cr[] crArray = cr.values();
        int n2 = crArray.length;
        for (int i2 = 0; i2 < n2; ++i2) {
            cr cr2;
            cr.l[cr2.g] = cr2 = crArray[i2];
        }
    }
}

