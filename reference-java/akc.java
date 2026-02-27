/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akc
extends akj
implements akw {
    public akc(boolean bl2) {
        super(bl2);
        this.A = true;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.bS;
    }

    @Override
    protected int b(int n2) {
        return 2;
    }

    @Override
    protected akj e() {
        return ajn.bV;
    }

    @Override
    protected akj i() {
        return ajn.bU;
    }

    @Override
    public int b() {
        return 37;
    }

    @Override
    protected boolean c(int n2) {
        return this.a || (n2 & 8) != 0;
    }

    @Override
    protected int f(ahl ahl2, int n2, int n3, int n4, int n5) {
        return this.e(ahl2, n2, n3, n4).a();
    }

    private int j(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (!this.d(n5)) {
            return this.h(ahb2, n2, n3, n4, n5);
        }
        return Math.max(this.h(ahb2, n2, n3, n4, n5) - this.h((ahl)ahb2, n2, n3, n4, n5), 0);
    }

    public boolean d(int n2) {
        return (n2 & 4) == 4;
    }

    @Override
    protected boolean a(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = this.h(ahb2, n2, n3, n4, n5);
        if (n6 >= 15) {
            return true;
        }
        if (n6 == 0) {
            return false;
        }
        int n7 = this.h((ahl)ahb2, n2, n3, n4, n5);
        if (n7 == 0) {
            return true;
        }
        return n6 >= n7;
    }

    @Override
    protected int h(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6;
        int n7 = super.h(ahb2, n2, n3, n4, n5);
        int n8 = akc.l(n5);
        int n9 = n2 + p.a[n8];
        aji aji2 = ahb2.a(n9, n3, n6 = n4 + p.b[n8]);
        if (aji2.M()) {
            n7 = aji2.g(ahb2, n9, n3, n6, p.f[n8]);
        } else if (n7 < 15 && aji2.r() && (aji2 = ahb2.a(n9 += p.a[n8], n3, n6 += p.b[n8])).M()) {
            n7 = aji2.g(ahb2, n9, n3, n6, p.f[n8]);
        }
        return n7;
    }

    public aoz e(ahl ahl2, int n2, int n3, int n4) {
        return (aoz)ahl2.o(n2, n3, n4);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        int n6 = ahb2.e(n2, n3, n4);
        boolean bl2 = this.a | (n6 & 8) != 0;
        boolean bl3 = !this.d(n6);
        int n7 = bl3 ? 4 : 0;
        ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "random.click", 0.3f, bl3 ? 0.55f : 0.5f);
        ahb2.a(n2, n3, n4, (n7 |= bl2 ? 8 : 0) | n6 & 3, 2);
        this.c(ahb2, n2, n3, n4, ahb2.s);
        return true;
    }

    @Override
    protected void b(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5;
        int n6;
        int n7;
        if (!(ahb2.a(n2, n3, n4, this) || (n7 = this.j(ahb2, n2, n3, n4, n6 = ahb2.e(n2, n3, n4))) == (n5 = this.e((ahl)ahb2, n2, n3, n4).a()) && this.c(n6) == this.a(ahb2, n2, n3, n4, n6))) {
            if (this.i(ahb2, n2, n3, n4, n6)) {
                ahb2.a(n2, n3, n4, this, this.b(0), -1);
            } else {
                ahb2.a(n2, n3, n4, this, this.b(0), 0);
            }
        }
    }

    private void c(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = this.j(ahb2, n2, n3, n4, n5);
        int n7 = this.e((ahl)ahb2, n2, n3, n4).a();
        this.e((ahl)ahb2, n2, n3, n4).a(n6);
        if (n7 != n6 || !this.d(n5)) {
            boolean bl2;
            boolean bl3 = this.a(ahb2, n2, n3, n4, n5);
            boolean bl4 = bl2 = this.a || (n5 & 8) != 0;
            if (bl2 && !bl3) {
                ahb2.a(n2, n3, n4, n5 & 0xFFFFFFF7, 2);
            } else if (!bl2 && bl3) {
                ahb2.a(n2, n3, n4, n5 | 8, 2);
            }
            this.e(ahb2, n2, n3, n4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (this.a) {
            int n5 = ahb2.e(n2, n3, n4);
            ahb2.d(n2, n3, n4, this.i(), n5 | 8, 4);
        }
        this.c(ahb2, n2, n3, n4, random);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        ahb2.a(n2, n3, n4, this.a(ahb2, 0));
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        super.a(ahb2, n2, n3, n4, aji2, n5);
        ahb2.p(n2, n3, n4);
        this.e(ahb2, n2, n3, n4);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        super.a(ahb2, n2, n3, n4, n5, n6);
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null) {
            return aor2.c(n5, n6);
        }
        return false;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new aoz();
    }
}

