/*
 * Decompiled with CFR 0.152.
 */
public class agu {
    public final int a;
    public final int b;

    public agu(int n2, int n3) {
        this.a = n2;
        this.b = n3;
    }

    public static long a(int n2, int n3) {
        return (long)n2 & 0xFFFFFFFFL | ((long)n3 & 0xFFFFFFFFL) << 32;
    }

    public int hashCode() {
        int n2 = 1664525 * this.a + 1013904223;
        int n3 = 1664525 * (this.b ^ 0xDEADBEEF) + 1013904223;
        return n2 ^ n3;
    }

    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }
        if (object instanceof agu) {
            agu agu2 = (agu)object;
            return this.a == agu2.a && this.b == agu2.b;
        }
        return false;
    }

    public int a() {
        return (this.a << 4) + 8;
    }

    public int b() {
        return (this.b << 4) + 8;
    }

    public agt a(int n2) {
        return new agt(this.a(), n2, this.b());
    }

    public String toString() {
        return "[" + this.a + ", " + this.b + "]";
    }
}

