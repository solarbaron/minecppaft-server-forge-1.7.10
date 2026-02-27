/*
 * Decompiled with CFR 0.152.
 */
public class ahm {
    public static final ahm[] a = new ahm[16];
    public static final ahm b = new ahm(0, "default", 1).i();
    public static final ahm c = new ahm(1, "flat");
    public static final ahm d = new ahm(2, "largeBiomes");
    public static final ahm e = new ahm(3, "amplified").j();
    public static final ahm f = new ahm(8, "default_1_1", 0).a(false);
    private final int g;
    private final String h;
    private final int i;
    private boolean j;
    private boolean k;
    private boolean l;

    private ahm(int n2, String string) {
        this(n2, string, 0);
    }

    private ahm(int n2, String string, int n3) {
        this.h = string;
        this.i = n3;
        this.j = true;
        this.g = n2;
        ahm.a[n2] = this;
    }

    public String a() {
        return this.h;
    }

    public int d() {
        return this.i;
    }

    public ahm a(int n2) {
        if (this == b && n2 == 0) {
            return f;
        }
        return this;
    }

    private ahm a(boolean bl2) {
        this.j = bl2;
        return this;
    }

    private ahm i() {
        this.k = true;
        return this;
    }

    public boolean f() {
        return this.k;
    }

    public static ahm a(String string) {
        for (int i2 = 0; i2 < a.length; ++i2) {
            if (a[i2] == null || !ahm.a[i2].h.equalsIgnoreCase(string)) continue;
            return a[i2];
        }
        return null;
    }

    public int g() {
        return this.g;
    }

    private ahm j() {
        this.l = true;
        return this;
    }
}

