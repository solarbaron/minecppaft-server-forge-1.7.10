/*
 * Decompiled with CFR 0.152.
 */
public class r
implements Comparable {
    public int a;
    public int b;
    public int c;

    public r() {
    }

    public r(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public r(r r2) {
        this.a = r2.a;
        this.b = r2.b;
        this.c = r2.c;
    }

    public boolean equals(Object object) {
        if (!(object instanceof r)) {
            return false;
        }
        r r2 = (r)object;
        return this.a == r2.a && this.b == r2.b && this.c == r2.c;
    }

    public int hashCode() {
        return this.a + this.c << 8 + this.b << 16;
    }

    public int a(r r2) {
        if (this.b == r2.b) {
            if (this.c == r2.c) {
                return this.a - r2.a;
            }
            return this.c - r2.c;
        }
        return this.b - r2.b;
    }

    public void b(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public float e(int n2, int n3, int n4) {
        float f2 = this.a - n2;
        float f3 = this.b - n3;
        float f4 = this.c - n4;
        return f2 * f2 + f3 * f3 + f4 * f4;
    }

    public float e(r r2) {
        return this.e(r2.a, r2.b, r2.c);
    }

    public String toString() {
        return "Pos{x=" + this.a + ", y=" + this.b + ", z=" + this.c + '}';
    }

    public /* synthetic */ int compareTo(Object object) {
        return this.a((r)object);
    }
}

