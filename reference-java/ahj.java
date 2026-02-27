/*
 * Decompiled with CFR 0.152.
 */
public final class ahj {
    private final long a;
    private final ahk b;
    private final boolean c;
    private final boolean d;
    private final ahm e;
    private boolean f;
    private boolean g;
    private String h = "";

    public ahj(long l2, ahk ahk2, boolean bl2, boolean bl3, ahm ahm2) {
        this.a = l2;
        this.b = ahk2;
        this.c = bl2;
        this.d = bl3;
        this.e = ahm2;
    }

    public ahj(ays ays2) {
        this(ays2.b(), ays2.r(), ays2.s(), ays2.t(), ays2.u());
    }

    public ahj a() {
        this.g = true;
        return this;
    }

    public ahj a(String string) {
        this.h = string;
        return this;
    }

    public boolean c() {
        return this.g;
    }

    public long d() {
        return this.a;
    }

    public ahk e() {
        return this.b;
    }

    public boolean f() {
        return this.d;
    }

    public boolean g() {
        return this.c;
    }

    public ahm h() {
        return this.e;
    }

    public boolean i() {
        return this.f;
    }

    public static ahk a(int n2) {
        return ahk.a(n2);
    }

    public String j() {
        return this.h;
    }
}

