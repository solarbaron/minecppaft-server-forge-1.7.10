/*
 * Decompiled with CFR 0.152.
 */
public class azx {
    private final bac a;
    private final String b;
    private final bah c;
    private String d;

    public azx(bac bac2, String string, bah bah2) {
        this.a = bac2;
        this.b = string;
        this.c = bah2;
        this.d = string;
    }

    public String b() {
        return this.b;
    }

    public bah c() {
        return this.c;
    }

    public String d() {
        return this.d;
    }

    public void a(String string) {
        this.d = string;
        this.a.b(this);
    }
}

