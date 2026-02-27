/*
 * Decompiled with CFR 0.152.
 */
public class apz {
    private int a;
    private int b;
    private int c;
    private byte[] d;
    private apv e;
    private apv f;
    private apv g;
    private apv h;

    public apz(int n2, boolean bl2) {
        this.a = n2;
        this.d = new byte[4096];
        this.f = new apv(this.d.length, 4);
        this.g = new apv(this.d.length, 4);
        if (bl2) {
            this.h = new apv(this.d.length, 4);
        }
    }

    public aji a(int n2, int n3, int n4) {
        int n5 = this.d[n3 << 8 | n4 << 4 | n2] & 0xFF;
        if (this.e != null) {
            n5 = this.e.a(n2, n3, n4) << 8 | n5;
        }
        return aji.e(n5);
    }

    public void a(int n2, int n3, int n4, aji aji2) {
        aji aji3;
        int n5 = this.d[n3 << 8 | n4 << 4 | n2] & 0xFF;
        if (this.e != null) {
            n5 = this.e.a(n2, n3, n4) << 8 | n5;
        }
        if ((aji3 = aji.e(n5)) != ajn.a) {
            --this.b;
            if (aji3.t()) {
                --this.c;
            }
        }
        if (aji2 != ajn.a) {
            ++this.b;
            if (aji2.t()) {
                ++this.c;
            }
        }
        int n6 = aji.b(aji2);
        this.d[n3 << 8 | n4 << 4 | n2] = (byte)(n6 & 0xFF);
        if (n6 > 255) {
            if (this.e == null) {
                this.e = new apv(this.d.length, 4);
            }
            this.e.a(n2, n3, n4, (n6 & 0xF00) >> 8);
        } else if (this.e != null) {
            this.e.a(n2, n3, n4, 0);
        }
    }

    public int b(int n2, int n3, int n4) {
        return this.f.a(n2, n3, n4);
    }

    public void a(int n2, int n3, int n4, int n5) {
        this.f.a(n2, n3, n4, n5);
    }

    public boolean a() {
        return this.b == 0;
    }

    public boolean b() {
        return this.c > 0;
    }

    public int d() {
        return this.a;
    }

    public void b(int n2, int n3, int n4, int n5) {
        this.h.a(n2, n3, n4, n5);
    }

    public int c(int n2, int n3, int n4) {
        return this.h.a(n2, n3, n4);
    }

    public void c(int n2, int n3, int n4, int n5) {
        this.g.a(n2, n3, n4, n5);
    }

    public int d(int n2, int n3, int n4) {
        return this.g.a(n2, n3, n4);
    }

    public void e() {
        this.b = 0;
        this.c = 0;
        for (int i2 = 0; i2 < 16; ++i2) {
            for (int i3 = 0; i3 < 16; ++i3) {
                for (int i4 = 0; i4 < 16; ++i4) {
                    aji aji2 = this.a(i2, i3, i4);
                    if (aji2 == ajn.a) continue;
                    ++this.b;
                    if (!aji2.t()) continue;
                    ++this.c;
                }
            }
        }
    }

    public byte[] g() {
        return this.d;
    }

    public apv i() {
        return this.e;
    }

    public apv j() {
        return this.f;
    }

    public apv k() {
        return this.g;
    }

    public apv l() {
        return this.h;
    }

    public void a(byte[] byArray) {
        this.d = byArray;
    }

    public void a(apv apv2) {
        this.e = apv2;
    }

    public void b(apv apv2) {
        this.f = apv2;
    }

    public void c(apv apv2) {
        this.g = apv2;
    }

    public void d(apv apv2) {
        this.h = apv2;
    }
}

