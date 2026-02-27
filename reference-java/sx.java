/*
 * Decompiled with CFR 0.152.
 */
public enum sx {
    a(yb.class, 70, awt.a, false, false),
    b(wf.class, 10, awt.a, true, true),
    c(wd.class, 15, awt.a, true, false),
    d(wu.class, 5, awt.h, true, false);

    private final Class e;
    private final int f;
    private final awt g;
    private final boolean h;
    private final boolean i;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private sx(awt awt2, boolean bl2, boolean bl3) {
        void var7_5;
        void var6_4;
        this.e = awt2;
        this.f = bl2 ? 1 : 0;
        this.g = (awt)bl3;
        this.h = var6_4;
        this.i = var7_5;
    }

    public Class a() {
        return this.e;
    }

    public int b() {
        return this.f;
    }

    public awt c() {
        return this.g;
    }

    public boolean d() {
        return this.h;
    }

    public boolean e() {
        return this.i;
    }
}

