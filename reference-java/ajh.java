/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ajh
extends akk {
    public static final int[][] a = new int[][]{{0, 1}, {-1, 0}, {0, -1}, {1, 0}};

    public ajh() {
        super(awt.n);
        this.e();
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        za za2;
        if (ahb2.E) {
            return true;
        }
        int n6 = ahb2.e(n2, n3, n4);
        if (!ajh.b(n6)) {
            int n7 = ajh.l(n6);
            if (ahb2.a(n2 += a[n7][0], n3, n4 += a[n7][1]) != this) {
                return true;
            }
            n6 = ahb2.e(n2, n3, n4);
        }
        if (!ahb2.t.e() || ahb2.a(n2, n4) == ahu.w) {
            double d2 = (double)n2 + 0.5;
            double d3 = (double)n3 + 0.5;
            double d4 = (double)n4 + 0.5;
            ahb2.f(n2, n3, n4);
            int n8 = ajh.l(n6);
            if (ahb2.a(n2 += a[n8][0], n3, n4 += a[n8][1]) == this) {
                ahb2.f(n2, n3, n4);
                d2 = (d2 + (double)n2 + 0.5) / 2.0;
                d3 = (d3 + (double)n3 + 0.5) / 2.0;
                d4 = (d4 + (double)n4 + 0.5) / 2.0;
            }
            ahb2.a(null, (double)((float)n2 + 0.5f), (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), 5.0f, true, true);
            return true;
        }
        if (ajh.c(n6)) {
            yz yz3 = null;
            for (yz yz4 : ahb2.h) {
                if (!yz4.bm()) continue;
                r r2 = yz4.bB;
                if (r2.a != n2 || r2.b != n3 || r2.c != n4) continue;
                yz3 = yz4;
            }
            if (yz3 == null) {
                ajh.a(ahb2, n2, n3, n4, false);
            } else {
                yz2.b(new fr("tile.bed.occupied", new Object[0]));
                return true;
            }
        }
        if ((za2 = yz2.a(n2, n3, n4)) == za.a) {
            ajh.a(ahb2, n2, n3, n4, true);
            return true;
        }
        if (za2 == za.c) {
            yz2.b(new fr("tile.bed.noSleep", new Object[0]));
        } else if (za2 == za.f) {
            yz2.b(new fr("tile.bed.notSafe", new Object[0]));
        }
        return true;
    }

    @Override
    public int b() {
        return 14;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.e();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = ajh.l(n5);
        if (ajh.b(n5)) {
            if (ahb2.a(n2 - a[n6][0], n3, n4 - a[n6][1]) != this) {
                ahb2.f(n2, n3, n4);
            }
        } else if (ahb2.a(n2 + a[n6][0], n3, n4 + a[n6][1]) != this) {
            ahb2.f(n2, n3, n4);
            if (!ahb2.E) {
                this.b(ahb2, n2, n3, n4, n5, 0);
            }
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (ajh.b(n2)) {
            return adb.d(0);
        }
        return ade.aV;
    }

    private void e() {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5625f, 1.0f);
    }

    public static boolean b(int n2) {
        return (n2 & 8) != 0;
    }

    public static boolean c(int n2) {
        return (n2 & 4) != 0;
    }

    public static void a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        int n5 = ahb2.e(n2, n3, n4);
        n5 = bl2 ? (n5 |= 4) : (n5 &= 0xFFFFFFFB);
        ahb2.a(n2, n3, n4, n5, 4);
    }

    public static r a(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = akk.l(n6);
        for (int i2 = 0; i2 <= 1; ++i2) {
            int n8 = n2 - a[n7][0] * i2 - 1;
            int n9 = n4 - a[n7][1] * i2 - 1;
            int n10 = n8 + 2;
            int n11 = n9 + 2;
            for (int i3 = n8; i3 <= n10; ++i3) {
                for (int i4 = n9; i4 <= n11; ++i4) {
                    if (!ahb.a(ahb2, i3, n3 - 1, i4) || ahb2.a(i3, n3, i4).o().k() || ahb2.a(i3, n3 + 1, i4).o().k()) continue;
                    if (n5 > 0) {
                        --n5;
                        continue;
                    }
                    return new r(i3, n3, i4);
                }
            }
        }
        return null;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (!ajh.b(n5)) {
            super.a(ahb2, n2, n3, n4, n5, f2, 0);
        }
    }

    @Override
    public int h() {
        return 1;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        int n6;
        if (yz2.bE.d && ajh.b(n5) && ahb2.a(n2 -= a[n6 = ajh.l(n5)][0], n3, n4 -= a[n6][1]) == this) {
            ahb2.f(n2, n3, n4);
        }
    }
}

