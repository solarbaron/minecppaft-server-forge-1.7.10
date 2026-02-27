/*
 * Decompiled with CFR 0.152.
 */
public enum zb {
    a(0, "options.chat.visibility.full"),
    b(1, "options.chat.visibility.system"),
    c(2, "options.chat.visibility.hidden");

    private static final zb[] d;
    private final int e;
    private final String f;

    /*
     * WARNING - void declaration
     */
    private zb() {
        void var4_1;
        void var3_2;
        this.e = var3_2;
        this.f = var4_1;
    }

    public int a() {
        return this.e;
    }

    public static zb a(int n2) {
        return d[n2 % d.length];
    }

    static {
        d = new zb[zb.values().length];
        zb[] zbArray = zb.values();
        int n2 = zbArray.length;
        for (int i2 = 0; i2 < n2; ++i2) {
            zb zb2;
            zb.d[zb2.e] = zb2 = zbArray[i2];
        }
    }
}

