/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class alb
extends aji {
    private int[] a = new int[256];
    private int[] b = new int[256];

    protected alb() {
        super(awt.o);
        this.a(true);
    }

    public static void e() {
        ajn.ab.a(alb.b(ajn.f), 5, 20);
        ajn.ab.a(alb.b(ajn.bw), 5, 20);
        ajn.ab.a(alb.b(ajn.bx), 5, 20);
        ajn.ab.a(alb.b(ajn.aJ), 5, 20);
        ajn.ab.a(alb.b(ajn.ad), 5, 20);
        ajn.ab.a(alb.b(ajn.bG), 5, 20);
        ajn.ab.a(alb.b(ajn.bF), 5, 20);
        ajn.ab.a(alb.b(ajn.bH), 5, 20);
        ajn.ab.a(alb.b(ajn.r), 5, 5);
        ajn.ab.a(alb.b(ajn.s), 5, 5);
        ajn.ab.a(alb.b(ajn.t), 30, 60);
        ajn.ab.a(alb.b(ajn.u), 30, 60);
        ajn.ab.a(alb.b(ajn.X), 30, 20);
        ajn.ab.a(alb.b(ajn.W), 15, 100);
        ajn.ab.a(alb.b(ajn.H), 60, 100);
        ajn.ab.a(alb.b(ajn.cm), 60, 100);
        ajn.ab.a(alb.b(ajn.N), 60, 100);
        ajn.ab.a(alb.b(ajn.O), 60, 100);
        ajn.ab.a(alb.b(ajn.L), 30, 60);
        ajn.ab.a(alb.b(ajn.bd), 15, 100);
        ajn.ab.a(alb.b(ajn.ci), 5, 5);
        ajn.ab.a(alb.b(ajn.cf), 60, 20);
        ajn.ab.a(alb.b(ajn.cg), 60, 20);
    }

    public void a(int n2, int n3, int n4) {
        this.a[n2] = n3;
        this.b[n2] = n4;
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 3;
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public int a(ahb ahb2) {
        return 30;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        boolean bl2;
        if (!ahb2.O().b("doFireTick")) {
            return;
        }
        boolean bl3 = bl2 = ahb2.a(n2, n3 - 1, n4) == ajn.aL;
        if (ahb2.t instanceof aqr && ahb2.a(n2, n3 - 1, n4) == ajn.h) {
            bl2 = true;
        }
        if (!this.c(ahb2, n2, n3, n4)) {
            ahb2.f(n2, n3, n4);
        }
        if (!bl2 && ahb2.Q() && (ahb2.y(n2, n3, n4) || ahb2.y(n2 - 1, n3, n4) || ahb2.y(n2 + 1, n3, n4) || ahb2.y(n2, n3, n4 - 1) || ahb2.y(n2, n3, n4 + 1))) {
            ahb2.f(n2, n3, n4);
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if (n5 < 15) {
            ahb2.a(n2, n3, n4, n5 + random.nextInt(3) / 2, 4);
        }
        ahb2.a(n2, n3, n4, this, this.a(ahb2) + random.nextInt(10));
        if (!bl2 && !this.e(ahb2, n2, n3, n4)) {
            if (!ahb.a(ahb2, n2, n3 - 1, n4) || n5 > 3) {
                ahb2.f(n2, n3, n4);
            }
            return;
        }
        if (!bl2 && !this.e((ahl)ahb2, n2, n3 - 1, n4) && n5 == 15 && random.nextInt(4) == 0) {
            ahb2.f(n2, n3, n4);
            return;
        }
        boolean bl4 = ahb2.z(n2, n3, n4);
        int n6 = 0;
        if (bl4) {
            n6 = -50;
        }
        this.a(ahb2, n2 + 1, n3, n4, 300 + n6, random, n5);
        this.a(ahb2, n2 - 1, n3, n4, 300 + n6, random, n5);
        this.a(ahb2, n2, n3 - 1, n4, 250 + n6, random, n5);
        this.a(ahb2, n2, n3 + 1, n4, 250 + n6, random, n5);
        this.a(ahb2, n2, n3, n4 - 1, 300 + n6, random, n5);
        this.a(ahb2, n2, n3, n4 + 1, 300 + n6, random, n5);
        for (int i2 = n2 - 1; i2 <= n2 + 1; ++i2) {
            for (int i3 = n4 - 1; i3 <= n4 + 1; ++i3) {
                for (int i4 = n3 - 1; i4 <= n3 + 4; ++i4) {
                    int n7;
                    if (i2 == n2 && i4 == n3 && i3 == n4) continue;
                    int n8 = 100;
                    if (i4 > n3 + 1) {
                        n8 += (i4 - (n3 + 1)) * 100;
                    }
                    if ((n7 = this.m(ahb2, i2, i4, i3)) <= 0) continue;
                    int n9 = (n7 + 40 + ahb2.r.a() * 7) / (n5 + 30);
                    if (bl4) {
                        n9 /= 2;
                    }
                    if (n9 <= 0 || random.nextInt(n8) > n9 || ahb2.Q() && ahb2.y(i2, i4, i3) || ahb2.y(i2 - 1, i4, n4) || ahb2.y(i2 + 1, i4, i3) || ahb2.y(i2, i4, i3 - 1) || ahb2.y(i2, i4, i3 + 1)) continue;
                    int n10 = n5 + random.nextInt(5) / 4;
                    if (n10 > 15) {
                        n10 = 15;
                    }
                    ahb2.d(i2, i4, i3, this, n10, 3);
                }
            }
        }
    }

    @Override
    public boolean L() {
        return false;
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5, Random random, int n6) {
        int n7 = this.b[aji.b(ahb2.a(n2, n3, n4))];
        if (random.nextInt(n5) < n7) {
            boolean bl2;
            boolean bl3 = bl2 = ahb2.a(n2, n3, n4) == ajn.W;
            if (random.nextInt(n6 + 10) < 5 && !ahb2.y(n2, n3, n4)) {
                int n8 = n6 + random.nextInt(5) / 4;
                if (n8 > 15) {
                    n8 = 15;
                }
                ahb2.d(n2, n3, n4, this, n8, 3);
            } else {
                ahb2.f(n2, n3, n4);
            }
            if (bl2) {
                ajn.W.b(ahb2, n2, n3, n4, 1);
            }
        }
    }

    private boolean e(ahb ahb2, int n2, int n3, int n4) {
        if (this.e((ahl)ahb2, n2 + 1, n3, n4)) {
            return true;
        }
        if (this.e((ahl)ahb2, n2 - 1, n3, n4)) {
            return true;
        }
        if (this.e((ahl)ahb2, n2, n3 - 1, n4)) {
            return true;
        }
        if (this.e((ahl)ahb2, n2, n3 + 1, n4)) {
            return true;
        }
        if (this.e((ahl)ahb2, n2, n3, n4 - 1)) {
            return true;
        }
        return this.e((ahl)ahb2, n2, n3, n4 + 1);
    }

    private int m(ahb ahb2, int n2, int n3, int n4) {
        int n5 = 0;
        if (!ahb2.c(n2, n3, n4)) {
            return 0;
        }
        n5 = this.a(ahb2, n2 + 1, n3, n4, n5);
        n5 = this.a(ahb2, n2 - 1, n3, n4, n5);
        n5 = this.a(ahb2, n2, n3 - 1, n4, n5);
        n5 = this.a(ahb2, n2, n3 + 1, n4, n5);
        n5 = this.a(ahb2, n2, n3, n4 - 1, n5);
        n5 = this.a(ahb2, n2, n3, n4 + 1, n5);
        return n5;
    }

    @Override
    public boolean v() {
        return false;
    }

    public boolean e(ahl ahl2, int n2, int n3, int n4) {
        return this.a[aji.b(ahl2.a(n2, n3, n4))] > 0;
    }

    public int a(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = this.a[aji.b(ahb2.a(n2, n3, n4))];
        if (n6 > n5) {
            return n6;
        }
        return n5;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return ahb.a(ahb2, n2, n3 - 1, n4) || this.e(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb.a(ahb2, n2, n3 - 1, n4) && !this.e(ahb2, n2, n3, n4)) {
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.t.i <= 0 && ajn.aO.e(ahb2, n2, n3, n4)) {
            return;
        }
        if (!ahb.a(ahb2, n2, n3 - 1, n4) && !this.e(ahb2, n2, n3, n4)) {
            ahb2.f(n2, n3, n4);
            return;
        }
        ahb2.a(n2, n3, n4, this, this.a(ahb2) + ahb2.s.nextInt(10));
    }

    @Override
    public awv f(int n2) {
        return awv.f;
    }
}

