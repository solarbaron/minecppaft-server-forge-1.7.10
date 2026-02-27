/*
 * Decompiled with CFR 0.152.
 */
public class aye {
    public final int a;
    public final int b;
    public final int c;
    private final int j;
    int d = -1;
    float e;
    float f;
    float g;
    aye h;
    public boolean i;

    public aye(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.j = aye.a(n2, n3, n4);
    }

    public static int a(int n2, int n3, int n4) {
        return n3 & 0xFF | (n2 & Short.MAX_VALUE) << 8 | (n4 & Short.MAX_VALUE) << 24 | (n2 < 0 ? Integer.MIN_VALUE : 0) | (n4 < 0 ? 32768 : 0);
    }

    public float a(aye aye2) {
        float f2 = aye2.a - this.a;
        float f3 = aye2.b - this.b;
        float f4 = aye2.c - this.c;
        return qh.c(f2 * f2 + f3 * f3 + f4 * f4);
    }

    public float b(aye aye2) {
        float f2 = aye2.a - this.a;
        float f3 = aye2.b - this.b;
        float f4 = aye2.c - this.c;
        return f2 * f2 + f3 * f3 + f4 * f4;
    }

    public boolean equals(Object object) {
        if (object instanceof aye) {
            aye aye2 = (aye)object;
            return this.j == aye2.j && this.a == aye2.a && this.b == aye2.b && this.c == aye2.c;
        }
        return false;
    }

    public int hashCode() {
        return this.j;
    }

    public boolean a() {
        return this.d >= 0;
    }

    public String toString() {
        return this.a + ", " + this.b + ", " + this.c;
    }
}

