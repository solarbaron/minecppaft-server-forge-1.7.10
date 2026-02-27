/*
 * Decompiled with CFR 0.152.
 */
class att {
    public Class a;
    public final int b;
    public int c;
    public int d;
    public boolean e;

    public att(Class clazz, int n2, int n3, boolean bl2) {
        this.a = clazz;
        this.b = n2;
        this.d = n3;
        this.e = bl2;
    }

    public att(Class clazz, int n2, int n3) {
        this(clazz, n2, n3, false);
    }

    public boolean a(int n2) {
        return this.d == 0 || this.c < this.d;
    }

    public boolean a() {
        return this.d == 0 || this.c < this.d;
    }
}

