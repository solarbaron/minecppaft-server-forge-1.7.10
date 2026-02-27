/*
 * Decompiled with CFR 0.152.
 */
public class ayf {
    private final aye[] a;
    private int b;
    private int c;

    public ayf(aye[] ayeArray) {
        this.a = ayeArray;
        this.c = ayeArray.length;
    }

    public void a() {
        ++this.b;
    }

    public boolean b() {
        return this.b >= this.c;
    }

    public aye c() {
        if (this.c > 0) {
            return this.a[this.c - 1];
        }
        return null;
    }

    public aye a(int n2) {
        return this.a[n2];
    }

    public int d() {
        return this.c;
    }

    public void b(int n2) {
        this.c = n2;
    }

    public int e() {
        return this.b;
    }

    public void c(int n2) {
        this.b = n2;
    }

    public azw a(sa sa2, int n2) {
        double d2 = (double)this.a[n2].a + (double)((int)(sa2.M + 1.0f)) * 0.5;
        double d3 = this.a[n2].b;
        double d4 = (double)this.a[n2].c + (double)((int)(sa2.M + 1.0f)) * 0.5;
        return azw.a(d2, d3, d4);
    }

    public azw a(sa sa2) {
        return this.a(sa2, this.b);
    }

    public boolean a(ayf ayf2) {
        if (ayf2 == null) {
            return false;
        }
        if (ayf2.a.length != this.a.length) {
            return false;
        }
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            if (this.a[i2].a == ayf2.a[i2].a && this.a[i2].b == ayf2.a[i2].b && this.a[i2].c == ayf2.a[i2].c) continue;
            return false;
        }
        return true;
    }

    public boolean b(azw azw2) {
        aye aye2 = this.c();
        if (aye2 == null) {
            return false;
        }
        return aye2.a == (int)azw2.a && aye2.c == (int)azw2.c;
    }
}

