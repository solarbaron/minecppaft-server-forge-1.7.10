/*
 * Decompiled with CFR 0.152.
 */
public class ahs
implements Comparable {
    private static long f;
    private final aji g;
    public int a;
    public int b;
    public int c;
    public long d;
    public int e;
    private long h = f++;

    public ahs(int n2, int n3, int n4, aji aji2) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.g = aji2;
    }

    public boolean equals(Object object) {
        if (object instanceof ahs) {
            ahs ahs2 = (ahs)object;
            return this.a == ahs2.a && this.b == ahs2.b && this.c == ahs2.c && aji.a(this.g, ahs2.g);
        }
        return false;
    }

    public int hashCode() {
        return (this.a * 1024 * 1024 + this.c * 1024 + this.b) * 256;
    }

    public ahs a(long l2) {
        this.d = l2;
        return this;
    }

    public void a(int n2) {
        this.e = n2;
    }

    public int a(ahs ahs2) {
        if (this.d < ahs2.d) {
            return -1;
        }
        if (this.d > ahs2.d) {
            return 1;
        }
        if (this.e != ahs2.e) {
            return this.e - ahs2.e;
        }
        if (this.h < ahs2.h) {
            return -1;
        }
        if (this.h > ahs2.h) {
            return 1;
        }
        return 0;
    }

    public String toString() {
        return aji.b(this.g) + ": (" + this.a + ", " + this.b + ", " + this.c + "), " + this.d + ", " + this.e + ", " + this.h;
    }

    public aji a() {
        return this.g;
    }

    public /* synthetic */ int compareTo(Object object) {
        return this.a((ahs)object);
    }
}

