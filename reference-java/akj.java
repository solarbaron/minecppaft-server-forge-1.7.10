/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class akj
extends akk {
    protected final boolean a;

    protected akj(boolean bl2) {
        super(awt.q);
        this.a = bl2;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
            return false;
        }
        return super.c(ahb2, n2, n3, n4);
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
            return false;
        }
        return super.j(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4);
        if (!this.g((ahl)ahb2, n2, n3, n4, n5)) {
            boolean bl2 = this.a(ahb2, n2, n3, n4, n5);
            if (this.a && !bl2) {
                ahb2.d(n2, n3, n4, this.i(), n5, 2);
            } else if (!this.a) {
                ahb2.d(n2, n3, n4, this.e(), n5, 2);
                if (!bl2) {
                    ahb2.a(n2, n3, n4, this.e(), this.k(n5), -1);
                }
            }
        }
    }

    @Override
    public int b() {
        return 36;
    }

    protected boolean c(int n2) {
        return this.a;
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        return this.b(ahl2, n2, n3, n4, n5);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        int n6 = ahl2.e(n2, n3, n4);
        if (!this.c(n6)) {
            return 0;
        }
        int n7 = akj.l(n6);
        if (n7 == 0 && n5 == 3) {
            return this.f(ahl2, n2, n3, n4, n6);
        }
        if (n7 == 1 && n5 == 4) {
            return this.f(ahl2, n2, n3, n4, n6);
        }
        if (n7 == 2 && n5 == 2) {
            return this.f(ahl2, n2, n3, n4, n6);
        }
        if (n7 == 3 && n5 == 5) {
            return this.f(ahl2, n2, n3, n4, n6);
        }
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
            ahb2.d(n2 + 1, n3, n4, this);
            ahb2.d(n2 - 1, n3, n4, this);
            ahb2.d(n2, n3, n4 + 1, this);
            ahb2.d(n2, n3, n4 - 1, this);
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.d(n2, n3 + 1, n4, this);
            return;
        }
        this.b(ahb2, n2, n3, n4, aji2);
    }

    protected void b(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = ahb2.e(n2, n3, n4);
        if (!this.g((ahl)ahb2, n2, n3, n4, n5)) {
            boolean bl2 = this.a(ahb2, n2, n3, n4, n5);
            if ((this.a && !bl2 || !this.a && bl2) && !ahb2.a(n2, n3, n4, this)) {
                int n6 = -1;
                if (this.i(ahb2, n2, n3, n4, n5)) {
                    n6 = -3;
                } else if (this.a) {
                    n6 = -2;
                }
                ahb2.a(n2, n3, n4, this, this.b(n5), n6);
            }
        }
    }

    public boolean g(ahl ahl2, int n2, int n3, int n4, int n5) {
        return false;
    }

    protected boolean a(ahb ahb2, int n2, int n3, int n4, int n5) {
        return this.h(ahb2, n2, n3, n4, n5) > 0;
    }

    protected int h(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6;
        int n7 = akj.l(n5);
        int n8 = n2 + p.a[n7];
        int n9 = ahb2.g(n8, n3, n6 = n4 + p.b[n7], p.d[n7]);
        if (n9 >= 15) {
            return n9;
        }
        return Math.max(n9, ahb2.a(n8, n3, n6) == ajn.af ? ahb2.e(n8, n3, n6) : 0);
    }

    protected int h(ahl ahl2, int n2, int n3, int n4, int n5) {
        int n6 = akj.l(n5);
        switch (n6) {
            case 0: 
            case 2: {
                return Math.max(this.i(ahl2, n2 - 1, n3, n4, 4), this.i(ahl2, n2 + 1, n3, n4, 5));
            }
            case 1: 
            case 3: {
                return Math.max(this.i(ahl2, n2, n3, n4 + 1, 3), this.i(ahl2, n2, n3, n4 - 1, 2));
            }
        }
        return 0;
    }

    protected int i(ahl ahl2, int n2, int n3, int n4, int n5) {
        aji aji2 = ahl2.a(n2, n3, n4);
        if (this.a(aji2)) {
            if (aji2 == ajn.af) {
                return ahl2.e(n2, n3, n4);
            }
            return ahl2.e(n2, n3, n4, n5);
        }
        return 0;
    }

    @Override
    public boolean f() {
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) + 2) % 4;
        ahb2.a(n2, n3, n4, n5, 3);
        boolean bl2 = this.a(ahb2, n2, n3, n4, n5);
        if (bl2) {
            ahb2.a(n2, n3, n4, this, 1);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        this.e(ahb2, n2, n3, n4);
    }

    protected void e(ahb ahb2, int n2, int n3, int n4) {
        int n5 = akj.l(ahb2.e(n2, n3, n4));
        if (n5 == 1) {
            ahb2.e(n2 + 1, n3, n4, this);
            ahb2.b(n2 + 1, n3, n4, this, 4);
        }
        if (n5 == 3) {
            ahb2.e(n2 - 1, n3, n4, this);
            ahb2.b(n2 - 1, n3, n4, this, 5);
        }
        if (n5 == 2) {
            ahb2.e(n2, n3, n4 + 1, this);
            ahb2.b(n2, n3, n4 + 1, this, 2);
        }
        if (n5 == 0) {
            ahb2.e(n2, n3, n4 - 1, this);
            ahb2.b(n2, n3, n4 - 1, this, 3);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (this.a) {
            ahb2.d(n2 + 1, n3, n4, this);
            ahb2.d(n2 - 1, n3, n4, this);
            ahb2.d(n2, n3, n4 + 1, this);
            ahb2.d(n2, n3, n4 - 1, this);
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.d(n2, n3 + 1, n4, this);
        }
        super.b(ahb2, n2, n3, n4, n5);
    }

    @Override
    public boolean c() {
        return false;
    }

    protected boolean a(aji aji2) {
        return aji2.f();
    }

    protected int f(ahl ahl2, int n2, int n3, int n4, int n5) {
        return 15;
    }

    public static boolean d(aji aji2) {
        return ajn.aR.e(aji2) || ajn.bU.e(aji2);
    }

    public boolean e(aji aji2) {
        return aji2 == this.e() || aji2 == this.i();
    }

    public boolean i(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = akj.l(n5);
        if (akj.d(ahb2.a(n2 - p.a[n6], n3, n4 - p.b[n6]))) {
            int n7 = ahb2.e(n2 - p.a[n6], n3, n4 - p.b[n6]);
            int n8 = akj.l(n7);
            return n8 != n6;
        }
        return false;
    }

    protected int k(int n2) {
        return this.b(n2);
    }

    protected abstract int b(int var1);

    protected abstract akj e();

    protected abstract akj i();

    @Override
    public boolean c(aji aji2) {
        return this.e(aji2);
    }
}

