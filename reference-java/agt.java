/*
 * Decompiled with CFR 0.152.
 */
public class agt {
    public final int a;
    public final int b;
    public final int c;

    public agt(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public agt(azw azw2) {
        this(qh.c(azw2.a), qh.c(azw2.b), qh.c(azw2.c));
    }

    public boolean equals(Object object) {
        if (object instanceof agt) {
            agt agt2 = (agt)object;
            return agt2.a == this.a && agt2.b == this.b && agt2.c == this.c;
        }
        return false;
    }

    public int hashCode() {
        return this.a * 8976890 + this.b * 981131 + this.c;
    }
}

