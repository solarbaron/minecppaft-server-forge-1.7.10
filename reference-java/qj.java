/*
 * Decompiled with CFR 0.152.
 */
public final class qj
implements Comparable {
    public double a;
    public double b;
    public String c;

    public qj(String string, double d2, double d3) {
        this.c = string;
        this.a = d2;
        this.b = d3;
    }

    public int a(qj qj2) {
        if (qj2.a < this.a) {
            return -1;
        }
        if (qj2.a > this.a) {
            return 1;
        }
        return qj2.c.compareTo(this.c);
    }

    public /* synthetic */ int compareTo(Object object) {
        return this.a((qj)object);
    }
}

