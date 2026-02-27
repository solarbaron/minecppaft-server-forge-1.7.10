/*
 * Decompiled with CFR 0.152.
 */
public class vy {
    public final int a;
    public final int b;
    public final int c;
    public final int d;
    public final int e;
    public int f;
    public boolean g;
    private int h;

    public vy(int n2, int n3, int n4, int n5, int n6, int n7) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = n6;
        this.f = n7;
    }

    public int b(int n2, int n3, int n4) {
        int n5 = n2 - this.a;
        int n6 = n3 - this.b;
        int n7 = n4 - this.c;
        return n5 * n5 + n6 * n6 + n7 * n7;
    }

    public int c(int n2, int n3, int n4) {
        int n5 = n2 - this.a - this.d;
        int n6 = n3 - this.b;
        int n7 = n4 - this.c - this.e;
        return n5 * n5 + n6 * n6 + n7 * n7;
    }

    public int a() {
        return this.a + this.d;
    }

    public int b() {
        return this.b;
    }

    public int c() {
        return this.c + this.e;
    }

    public boolean a(int n2, int n3) {
        int n4 = n2 - this.a;
        int n5 = n3 - this.c;
        return n4 * this.d + n5 * this.e >= 0;
    }

    public void d() {
        this.h = 0;
    }

    public void e() {
        ++this.h;
    }

    public int f() {
        return this.h;
    }
}

