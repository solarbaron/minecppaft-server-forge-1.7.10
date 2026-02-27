/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ajz
extends akk
implements ajo {
    public ajz() {
        super(awt.k);
        this.a(true);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5;
        int n6;
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.d(n2, n3, n4, ajz.e(0), 0, 2);
        } else if (ahb2.s.nextInt(5) == 0 && (n6 = ajz.c(n5 = ahb2.e(n2, n3, n4))) < 2) {
            ahb2.a(n2, n3, n4, ++n6 << 2 | ajz.l(n5), 2);
        }
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        int n5;
        aji aji2;
        return (aji2 = ahb2.a(n2 += p.a[n5 = ajz.l(ahb2.e(n2, n3, n4))], n3, n4 += p.b[n5])) == ajn.r && alx.c(ahb2.e(n2, n3, n4)) == 3;
    }

    @Override
    public int b() {
        return 28;
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
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        int n6 = ajz.l(n5);
        int n7 = ajz.c(n5);
        int n8 = 4 + n7 * 2;
        int n9 = 5 + n7 * 2;
        float f2 = (float)n8 / 2.0f;
        switch (n6) {
            case 0: {
                this.a((8.0f - f2) / 16.0f, (12.0f - (float)n9) / 16.0f, (15.0f - (float)n8) / 16.0f, (8.0f + f2) / 16.0f, 0.75f, 0.9375f);
                break;
            }
            case 2: {
                this.a((8.0f - f2) / 16.0f, (12.0f - (float)n9) / 16.0f, 0.0625f, (8.0f + f2) / 16.0f, 0.75f, (1.0f + (float)n8) / 16.0f);
                break;
            }
            case 1: {
                this.a(0.0625f, (12.0f - (float)n9) / 16.0f, (8.0f - f2) / 16.0f, (1.0f + (float)n8) / 16.0f, 0.75f, (8.0f + f2) / 16.0f);
                break;
            }
            case 3: {
                this.a((15.0f - (float)n8) / 16.0f, (12.0f - (float)n9) / 16.0f, (8.0f - f2) / 16.0f, 0.9375f, 0.75f, (8.0f + f2) / 16.0f);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) + 0) % 4;
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        if (n5 == 1 || n5 == 0) {
            n5 = 2;
        }
        return p.f[p.e[n5]];
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!this.j(ahb2, n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.d(n2, n3, n4, ajz.e(0), 0, 2);
        }
    }

    public static int c(int n2) {
        return (n2 & 0xC) >> 2;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        int n7 = ajz.c(n5);
        int n8 = 1;
        if (n7 >= 2) {
            n8 = 3;
        }
        for (int i2 = 0; i2 < n8; ++i2) {
            this.a(ahb2, n2, n3, n4, new add(ade.aR, 1, 3));
        }
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        return 3;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = ajz.c(n5);
        return n6 < 2;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = akk.l(n5);
        int n7 = ajz.c(n5);
        ahb2.a(n2, n3, n4, ++n7 << 2 | n6, 2);
    }
}

