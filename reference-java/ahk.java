/*
 * Decompiled with CFR 0.152.
 */
public enum ahk {
    a(-1, ""),
    b(0, "survival"),
    c(1, "creative"),
    d(2, "adventure");

    int e;
    String f;

    /*
     * WARNING - void declaration
     */
    private ahk() {
        void var4_1;
        void var3_2;
        this.e = var3_2;
        this.f = var4_1;
    }

    public int a() {
        return this.e;
    }

    public String b() {
        return this.f;
    }

    public void a(yw yw2) {
        if (this == c) {
            yw2.c = true;
            yw2.d = true;
            yw2.a = true;
        } else {
            yw2.c = false;
            yw2.d = false;
            yw2.a = false;
            yw2.b = false;
        }
        yw2.e = !this.c();
    }

    public boolean c() {
        return this == d;
    }

    public boolean d() {
        return this == c;
    }

    public static ahk a(int n2) {
        for (ahk ahk2 : ahk.values()) {
            if (ahk2.e != n2) continue;
            return ahk2;
        }
        return b;
    }
}

