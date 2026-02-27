/*
 * Decompiled with CFR 0.152.
 */
public class amq {
    private final ahb a;
    private final int b;
    private final int c;
    private final int d;
    private int e = 0;
    private r f;
    private int g;
    private int h;

    public amq(ahb ahb2, int n2, int n3, int n4, int n5) {
        this.a = ahb2;
        this.b = n5;
        this.d = amp.a[n5][0];
        this.c = amp.a[n5][1];
        int n6 = n3;
        while (n3 > n6 - 21 && n3 > 0 && this.a(ahb2.a(n2, n3 - 1, n4))) {
            --n3;
        }
        int n7 = this.a(n2, n3, n4, this.d) - 1;
        if (n7 >= 0) {
            this.f = new r(n2 + n7 * p.a[this.d], n3, n4 + n7 * p.b[this.d]);
            this.h = this.a(this.f.a, this.f.b, this.f.c, this.c);
            if (this.h < 2 || this.h > 21) {
                this.f = null;
                this.h = 0;
            }
        }
        if (this.f != null) {
            this.g = this.a();
        }
    }

    protected int a(int n2, int n3, int n4, int n5) {
        aji aji2;
        aji aji3;
        int n6;
        int n7 = p.a[n5];
        int n8 = p.b[n5];
        for (n6 = 0; n6 < 22 && this.a(aji3 = this.a.a(n2 + n7 * n6, n3, n4 + n8 * n6)) && (aji2 = this.a.a(n2 + n7 * n6, n3 - 1, n4 + n8 * n6)) == ajn.Z; ++n6) {
        }
        aji3 = this.a.a(n2 + n7 * n6, n3, n4 + n8 * n6);
        if (aji3 == ajn.Z) {
            return n6;
        }
        return 0;
    }

    protected int a() {
        int n2;
        int n3;
        int n4;
        int n5;
        this.g = 0;
        block0: while (this.g < 21) {
            n5 = this.f.b + this.g;
            for (n4 = 0; n4 < this.h; ++n4) {
                n3 = this.f.a + n4 * p.a[amp.a[this.b][1]];
                n2 = this.f.c + n4 * p.b[amp.a[this.b][1]];
                aji aji2 = this.a.a(n3, n5, n2);
                if (!this.a(aji2)) break block0;
                if (aji2 == ajn.aO) {
                    ++this.e;
                }
                if (n4 == 0 ? (aji2 = this.a.a(n3 + p.a[amp.a[this.b][0]], n5, n2 + p.b[amp.a[this.b][0]])) != ajn.Z : n4 == this.h - 1 && (aji2 = this.a.a(n3 + p.a[amp.a[this.b][1]], n5, n2 + p.b[amp.a[this.b][1]])) != ajn.Z) break block0;
            }
            ++this.g;
        }
        for (n5 = 0; n5 < this.h; ++n5) {
            n4 = this.f.a + n5 * p.a[amp.a[this.b][1]];
            n3 = this.f.b + this.g;
            n2 = this.f.c + n5 * p.b[amp.a[this.b][1]];
            if (this.a.a(n4, n3, n2) == ajn.Z) continue;
            this.g = 0;
            break;
        }
        if (this.g > 21 || this.g < 3) {
            this.f = null;
            this.h = 0;
            this.g = 0;
            return 0;
        }
        return this.g;
    }

    protected boolean a(aji aji2) {
        return aji2.J == awt.a || aji2 == ajn.ab || aji2 == ajn.aO;
    }

    public boolean b() {
        return this.f != null && this.h >= 2 && this.h <= 21 && this.g >= 3 && this.g <= 21;
    }

    public void c() {
        for (int i2 = 0; i2 < this.h; ++i2) {
            int n2 = this.f.a + p.a[this.c] * i2;
            int n3 = this.f.c + p.b[this.c] * i2;
            for (int i3 = 0; i3 < this.g; ++i3) {
                int n4 = this.f.b + i3;
                this.a.d(n2, n4, n3, ajn.aO, this.b, 2);
            }
        }
    }

    static /* synthetic */ int a(amq amq2) {
        return amq2.e;
    }

    static /* synthetic */ int b(amq amq2) {
        return amq2.h;
    }

    static /* synthetic */ int c(amq amq2) {
        return amq2.g;
    }
}

