/*
 * Decompiled with CFR 0.152.
 */
public class mx {
    public ahb a;
    public mw b;
    private ahk c = ahk.a;
    private boolean d;
    private int e;
    private int f;
    private int g;
    private int h;
    private int i;
    private boolean j;
    private int k;
    private int l;
    private int m;
    private int n;
    private int o = -1;

    public mx(ahb ahb2) {
        this.a = ahb2;
    }

    public void a(ahk ahk2) {
        this.c = ahk2;
        ahk2.a(this.b.bE);
        this.b.q();
    }

    public ahk b() {
        return this.c;
    }

    public boolean d() {
        return this.c.d();
    }

    public void b(ahk ahk2) {
        if (this.c == ahk.a) {
            this.c = ahk2;
        }
        this.a(this.c);
    }

    public void a() {
        ++this.i;
        if (this.j) {
            int n2 = this.i - this.n;
            aji aji2 = this.a.a(this.k, this.l, this.m);
            if (aji2.o() == awt.a) {
                this.j = false;
            } else {
                float f2 = aji2.a(this.b, this.b.o, this.k, this.l, this.m) * (float)(n2 + 1);
                int n3 = (int)(f2 * 10.0f);
                if (n3 != this.o) {
                    this.a.d(this.b.y(), this.k, this.l, this.m, n3);
                    this.o = n3;
                }
                if (f2 >= 1.0f) {
                    this.j = false;
                    this.b(this.k, this.l, this.m);
                }
            }
        } else if (this.d) {
            aji aji3 = this.a.a(this.f, this.g, this.h);
            if (aji3.o() == awt.a) {
                this.a.d(this.b.y(), this.f, this.g, this.h, -1);
                this.o = -1;
                this.d = false;
            } else {
                int n4 = this.i - this.e;
                float f3 = aji3.a(this.b, this.b.o, this.f, this.g, this.h) * (float)(n4 + 1);
                int n5 = (int)(f3 * 10.0f);
                if (n5 != this.o) {
                    this.a.d(this.b.y(), this.f, this.g, this.h, n5);
                    this.o = n5;
                }
            }
        }
    }

    public void a(int n2, int n3, int n4, int n5) {
        if (this.c.c() && !this.b.d(n2, n3, n4)) {
            return;
        }
        if (this.d()) {
            if (!this.a.a(null, n2, n3, n4, n5)) {
                this.b(n2, n3, n4);
            }
            return;
        }
        this.a.a(null, n2, n3, n4, n5);
        this.e = this.i;
        float f2 = 1.0f;
        aji aji2 = this.a.a(n2, n3, n4);
        if (aji2.o() != awt.a) {
            aji2.a(this.a, n2, n3, n4, this.b);
            f2 = aji2.a(this.b, this.b.o, n2, n3, n4);
        }
        if (aji2.o() != awt.a && f2 >= 1.0f) {
            this.b(n2, n3, n4);
        } else {
            this.d = true;
            this.f = n2;
            this.g = n3;
            this.h = n4;
            int n6 = (int)(f2 * 10.0f);
            this.a.d(this.b.y(), n2, n3, n4, n6);
            this.o = n6;
        }
    }

    public void a(int n2, int n3, int n4) {
        if (n2 == this.f && n3 == this.g && n4 == this.h) {
            int n5 = this.i - this.e;
            aji aji2 = this.a.a(n2, n3, n4);
            if (aji2.o() != awt.a) {
                float f2 = aji2.a(this.b, this.b.o, n2, n3, n4) * (float)(n5 + 1);
                if (f2 >= 0.7f) {
                    this.d = false;
                    this.a.d(this.b.y(), n2, n3, n4, -1);
                    this.b(n2, n3, n4);
                } else if (!this.j) {
                    this.d = false;
                    this.j = true;
                    this.k = n2;
                    this.l = n3;
                    this.m = n4;
                    this.n = this.e;
                }
            }
        }
    }

    public void c(int n2, int n3, int n4) {
        this.d = false;
        this.a.d(this.b.y(), this.f, this.g, this.h, -1);
    }

    private boolean d(int n2, int n3, int n4) {
        aji aji2 = this.a.a(n2, n3, n4);
        int n5 = this.a.e(n2, n3, n4);
        aji2.a(this.a, n2, n3, n4, n5, this.b);
        boolean bl2 = this.a.f(n2, n3, n4);
        if (bl2) {
            aji2.b(this.a, n2, n3, n4, n5);
        }
        return bl2;
    }

    public boolean b(int n2, int n3, int n4) {
        if (this.c.c() && !this.b.d(n2, n3, n4)) {
            return false;
        }
        if (this.c.d() && this.b.be() != null && this.b.be().b() instanceof aeh) {
            return false;
        }
        aji aji2 = this.a.a(n2, n3, n4);
        int n5 = this.a.e(n2, n3, n4);
        this.a.a(this.b, 2001, n2, n3, n4, aji.b(aji2) + (this.a.e(n2, n3, n4) << 12));
        boolean bl2 = this.d(n2, n3, n4);
        if (this.d()) {
            this.b.a.a(new gh(n2, n3, n4, this.a));
        } else {
            add add2 = this.b.bF();
            boolean bl3 = this.b.a(aji2);
            if (add2 != null) {
                add2.a(this.a, aji2, n2, n3, n4, this.b);
                if (add2.b == 0) {
                    this.b.bG();
                }
            }
            if (bl2 && bl3) {
                aji2.a(this.a, this.b, n2, n3, n4, n5);
            }
        }
        return bl2;
    }

    public boolean a(yz yz2, ahb ahb2, add add2) {
        int n2 = add2.b;
        int n3 = add2.k();
        add add3 = add2.a(ahb2, yz2);
        if (add3 != add2 || add3 != null && (add3.b != n2 || add3.n() > 0 || add3.k() != n3)) {
            yz2.bm.a[yz2.bm.c] = add3;
            if (this.d()) {
                add3.b = n2;
                if (add3.g()) {
                    add3.b(n3);
                }
            }
            if (add3.b == 0) {
                yz2.bm.a[yz2.bm.c] = null;
            }
            if (!yz2.by()) {
                ((mw)yz2).a(yz2.bn);
            }
            return true;
        }
        return false;
    }

    public boolean a(yz yz2, ahb ahb2, add add2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if ((!yz2.an() || yz2.be() == null) && ahb2.a(n2, n3, n4).a(ahb2, n2, n3, n4, yz2, n5, f2, f3, f4)) {
            return true;
        }
        if (add2 == null) {
            return false;
        }
        if (this.d()) {
            int n6 = add2.k();
            int n7 = add2.b;
            boolean bl2 = add2.a(yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
            add2.b(n6);
            add2.b = n7;
            return bl2;
        }
        return add2.a(yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
    }

    public void a(mt mt2) {
        this.a = mt2;
    }
}

