/*
 * Decompiled with CFR 0.152.
 */
public class azu {
    public azv a;
    public int b;
    public int c;
    public int d;
    public int e;
    public azw f;
    public sa g;

    public azu(int n2, int n3, int n4, int n5, azw azw2) {
        this(n2, n3, n4, n5, azw2, true);
    }

    public azu(int n2, int n3, int n4, int n5, azw azw2, boolean bl2) {
        this.a = bl2 ? azv.b : azv.a;
        this.b = n2;
        this.c = n3;
        this.d = n4;
        this.e = n5;
        this.f = azw.a(azw2.a, azw2.b, azw2.c);
    }

    public azu(sa sa2) {
        this(sa2, azw.a(sa2.s, sa2.t, sa2.u));
    }

    public azu(sa sa2, azw azw2) {
        this.a = azv.c;
        this.g = sa2;
        this.f = azw2;
    }

    public String toString() {
        return "HitResult{type=" + (Object)((Object)this.a) + ", x=" + this.b + ", y=" + this.c + ", z=" + this.d + ", f=" + this.e + ", pos=" + this.f + ", entity=" + this.g + '}';
    }
}

