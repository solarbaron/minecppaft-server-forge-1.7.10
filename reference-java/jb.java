/*
 * Decompiled with CFR 0.152.
 */
public enum jb {
    a(0),
    b(1);

    private static final jb[] c;
    private final int d;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private jb() {
        void var3_2;
        this.d = var3_2;
    }

    static /* synthetic */ jb[] a() {
        return c;
    }

    static /* synthetic */ int a(jb jb2) {
        return jb2.d;
    }

    static {
        c = new jb[jb.values().length];
        jb[] jbArray = jb.values();
        int n2 = jbArray.length;
        for (int i2 = 0; i2 < n2; ++i2) {
            jb jb2;
            jb.c[jb2.d] = jb2 = jbArray[i2];
        }
    }
}

