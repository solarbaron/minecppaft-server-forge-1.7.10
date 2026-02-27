/*
 * Decompiled with CFR 0.152.
 */
public class ags {
    private int a;
    private int b;
    private int c;
    private aji d;
    private int e;
    private int f;

    public ags(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.e = n5;
        this.f = n6;
        this.d = aji2;
    }

    public int a() {
        return this.a;
    }

    public int b() {
        return this.b;
    }

    public int c() {
        return this.c;
    }

    public int d() {
        return this.e;
    }

    public int e() {
        return this.f;
    }

    public aji f() {
        return this.d;
    }

    public boolean equals(Object object) {
        if (object instanceof ags) {
            ags ags2 = (ags)object;
            return this.a == ags2.a && this.b == ags2.b && this.c == ags2.c && this.e == ags2.e && this.f == ags2.f && this.d == ags2.d;
        }
        return false;
    }

    public String toString() {
        return "TE(" + this.a + "," + this.b + "," + this.c + ")," + this.e + "," + this.f + "," + this.d;
    }
}

