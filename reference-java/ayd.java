/*
 * Decompiled with CFR 0.152.
 */
public class ayd {
    private aye[] a = new aye[1024];
    private int b;

    public aye a(aye aye2) {
        if (aye2.d >= 0) {
            throw new IllegalStateException("OW KNOWS!");
        }
        if (this.b == this.a.length) {
            aye[] ayeArray = new aye[this.b << 1];
            System.arraycopy(this.a, 0, ayeArray, 0, this.b);
            this.a = ayeArray;
        }
        this.a[this.b] = aye2;
        aye2.d = this.b;
        this.a(this.b++);
        return aye2;
    }

    public void a() {
        this.b = 0;
    }

    public aye c() {
        aye aye2 = this.a[0];
        this.a[0] = this.a[--this.b];
        this.a[this.b] = null;
        if (this.b > 0) {
            this.b(0);
        }
        aye2.d = -1;
        return aye2;
    }

    public void a(aye aye2, float f2) {
        float f3 = aye2.g;
        aye2.g = f2;
        if (f2 < f3) {
            this.a(aye2.d);
        } else {
            this.b(aye2.d);
        }
    }

    private void a(int n2) {
        aye aye2 = this.a[n2];
        float f2 = aye2.g;
        while (n2 > 0) {
            int n3 = n2 - 1 >> 1;
            aye aye3 = this.a[n3];
            if (!(f2 < aye3.g)) break;
            this.a[n2] = aye3;
            aye3.d = n2;
            n2 = n3;
        }
        this.a[n2] = aye2;
        aye2.d = n2;
    }

    private void b(int n2) {
        aye aye2 = this.a[n2];
        float f2 = aye2.g;
        while (true) {
            float f3;
            aye aye3;
            int n3 = 1 + (n2 << 1);
            int n4 = n3 + 1;
            if (n3 >= this.b) break;
            aye aye4 = this.a[n3];
            float f4 = aye4.g;
            if (n4 >= this.b) {
                aye3 = null;
                f3 = Float.POSITIVE_INFINITY;
            } else {
                aye3 = this.a[n4];
                f3 = aye3.g;
            }
            if (f4 < f3) {
                if (!(f4 < f2)) break;
                this.a[n2] = aye4;
                aye4.d = n2;
                n2 = n3;
                continue;
            }
            if (!(f3 < f2)) break;
            this.a[n2] = aye3;
            aye3.d = n2;
            n2 = n4;
        }
        this.a[n2] = aye2;
        aye2.d = n2;
    }

    public boolean e() {
        return this.b == 0;
    }
}

