/*
 * Decompiled with CFR 0.152.
 */
public enum abd {
    a(5, new int[]{1, 3, 2, 1}, 15),
    b(15, new int[]{2, 5, 4, 1}, 12),
    c(15, new int[]{2, 6, 5, 2}, 9),
    d(7, new int[]{2, 5, 3, 1}, 25),
    e(33, new int[]{3, 8, 6, 3}, 10);

    private int f;
    private int[] g;
    private int h;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private abd(int n3) {
        void var5_3;
        void var4_2;
        this.f = n3;
        this.g = var4_2;
        this.h = var5_3;
    }

    public int a(int n2) {
        return abb.e()[n2] * this.f;
    }

    public int b(int n2) {
        return this.g[n2];
    }

    public int a() {
        return this.h;
    }

    public adb b() {
        if (this == a) {
            return ade.aA;
        }
        if (this == b) {
            return ade.j;
        }
        if (this == d) {
            return ade.k;
        }
        if (this == c) {
            return ade.j;
        }
        if (this == e) {
            return ade.i;
        }
        return null;
    }
}

