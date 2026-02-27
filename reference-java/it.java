/*
 * Decompiled with CFR 0.152.
 */
public enum it {
    a(0),
    b(1),
    c(2);

    private final int d;
    private static final it[] e;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private it() {
        void var3_2;
        this.d = var3_2;
    }

    static /* synthetic */ it[] a() {
        return e;
    }

    static /* synthetic */ int a(it it2) {
        return it2.d;
    }

    static {
        e = new it[it.values().length];
        it[] itArray = it.values();
        int n2 = itArray.length;
        for (int i2 = 0; i2 < n2; ++i2) {
            it it2;
            it.e[it2.d] = it2 = itArray[i2];
        }
    }
}

