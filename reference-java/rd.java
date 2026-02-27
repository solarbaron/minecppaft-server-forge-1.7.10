/*
 * Decompiled with CFR 0.152.
 */
public enum rd {
    a(0, "options.difficulty.peaceful"),
    b(1, "options.difficulty.easy"),
    c(2, "options.difficulty.normal"),
    d(3, "options.difficulty.hard");

    private static final rd[] e;
    private final int f;
    private final String g;

    /*
     * WARNING - void declaration
     */
    private rd() {
        void var4_1;
        void var3_2;
        this.f = var3_2;
        this.g = var4_1;
    }

    public int a() {
        return this.f;
    }

    public static rd a(int n2) {
        return e[n2 % e.length];
    }

    public String b() {
        return this.g;
    }

    static {
        e = new rd[rd.values().length];
        rd[] rdArray = rd.values();
        int n2 = rdArray.length;
        for (int i2 = 0; i2 < n2; ++i2) {
            rd rd2;
            rd.e[rd2.f] = rd2 = rdArray[i2];
        }
    }
}

