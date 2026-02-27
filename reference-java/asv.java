/*
 * Decompiled with CFR 0.152.
 */
public class asv {
    public int a;
    public int b;
    public int c;
    public int d;
    public int e;
    public int f;

    public asv() {
    }

    public asv(int[] nArray) {
        if (nArray.length == 6) {
            this.a = nArray[0];
            this.b = nArray[1];
            this.c = nArray[2];
            this.d = nArray[3];
            this.e = nArray[4];
            this.f = nArray[5];
        }
    }

    public static asv a() {
        return new asv(Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE, Integer.MIN_VALUE);
    }

    public static asv a(int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9, int n10, int n11) {
        switch (n11) {
            default: {
                return new asv(n2 + n5, n3 + n6, n4 + n7, n2 + n8 - 1 + n5, n3 + n9 - 1 + n6, n4 + n10 - 1 + n7);
            }
            case 2: {
                return new asv(n2 + n5, n3 + n6, n4 - n10 + 1 + n7, n2 + n8 - 1 + n5, n3 + n9 - 1 + n6, n4 + n7);
            }
            case 0: {
                return new asv(n2 + n5, n3 + n6, n4 + n7, n2 + n8 - 1 + n5, n3 + n9 - 1 + n6, n4 + n10 - 1 + n7);
            }
            case 1: {
                return new asv(n2 - n10 + 1 + n7, n3 + n6, n4 + n5, n2 + n7, n3 + n9 - 1 + n6, n4 + n8 - 1 + n5);
            }
            case 3: 
        }
        return new asv(n2 + n7, n3 + n6, n4 + n5, n2 + n10 - 1 + n7, n3 + n9 - 1 + n6, n4 + n8 - 1 + n5);
    }

    public asv(asv asv2) {
        this.a = asv2.a;
        this.b = asv2.b;
        this.c = asv2.c;
        this.d = asv2.d;
        this.e = asv2.e;
        this.f = asv2.f;
    }

    public asv(int n2, int n3, int n4, int n5, int n6, int n7) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = n6;
        this.f = n7;
    }

    public asv(int n2, int n3, int n4, int n5) {
        this.a = n2;
        this.c = n3;
        this.d = n4;
        this.f = n5;
        this.b = 1;
        this.e = 512;
    }

    public boolean a(asv asv2) {
        return this.d >= asv2.a && this.a <= asv2.d && this.f >= asv2.c && this.c <= asv2.f && this.e >= asv2.b && this.b <= asv2.e;
    }

    public boolean a(int n2, int n3, int n4, int n5) {
        return this.d >= n2 && this.a <= n4 && this.f >= n3 && this.c <= n5;
    }

    public void b(asv asv2) {
        this.a = Math.min(this.a, asv2.a);
        this.b = Math.min(this.b, asv2.b);
        this.c = Math.min(this.c, asv2.c);
        this.d = Math.max(this.d, asv2.d);
        this.e = Math.max(this.e, asv2.e);
        this.f = Math.max(this.f, asv2.f);
    }

    public void a(int n2, int n3, int n4) {
        this.a += n2;
        this.b += n3;
        this.c += n4;
        this.d += n2;
        this.e += n3;
        this.f += n4;
    }

    public boolean b(int n2, int n3, int n4) {
        return n2 >= this.a && n2 <= this.d && n4 >= this.c && n4 <= this.f && n3 >= this.b && n3 <= this.e;
    }

    public int b() {
        return this.d - this.a + 1;
    }

    public int c() {
        return this.e - this.b + 1;
    }

    public int d() {
        return this.f - this.c + 1;
    }

    public int e() {
        return this.a + (this.d - this.a + 1) / 2;
    }

    public int f() {
        return this.b + (this.e - this.b + 1) / 2;
    }

    public int g() {
        return this.c + (this.f - this.c + 1) / 2;
    }

    public String toString() {
        return "(" + this.a + ", " + this.b + ", " + this.c + "; " + this.d + ", " + this.e + ", " + this.f + ")";
    }

    public dn h() {
        return new dn(new int[]{this.a, this.b, this.c, this.d, this.e, this.f});
    }
}

