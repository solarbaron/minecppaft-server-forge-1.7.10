/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

public class ana
extends aji {
    private boolean a = true;
    private Set b = new HashSet();

    public ana() {
        super(awt.q);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.0625f, 1.0f);
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
        return 5;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return ahb.a(ahb2, n2, n3 - 1, n4) || ahb2.a(n2, n3 - 1, n4) == ajn.aN;
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        this.a(ahb2, n2, n3, n4, n2, n3, n4);
        ArrayList arrayList = new ArrayList(this.b);
        this.b.clear();
        for (int i2 = 0; i2 < arrayList.size(); ++i2) {
            agt agt2 = (agt)arrayList.get(i2);
            ahb2.d(agt2.a, agt2.b, agt2.c, this);
        }
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5, int n6, int n7) {
        int n8 = ahb2.e(n2, n3, n4);
        int n9 = 0;
        n9 = this.a(ahb2, n5, n6, n7, n9);
        this.a = false;
        int n10 = ahb2.w(n2, n3, n4);
        this.a = true;
        if (n10 > 0 && n10 > n9 - 1) {
            n9 = n10;
        }
        int n11 = 0;
        for (int i2 = 0; i2 < 4; ++i2) {
            int n12 = n2;
            int n13 = n4;
            if (i2 == 0) {
                --n12;
            }
            if (i2 == 1) {
                ++n12;
            }
            if (i2 == 2) {
                --n13;
            }
            if (i2 == 3) {
                ++n13;
            }
            if (n12 != n5 || n13 != n7) {
                n11 = this.a(ahb2, n12, n3, n13, n11);
            }
            if (ahb2.a(n12, n3, n13).r() && !ahb2.a(n2, n3 + 1, n4).r()) {
                if (n12 == n5 && n13 == n7 || n3 < n6) continue;
                n11 = this.a(ahb2, n12, n3 + 1, n13, n11);
                continue;
            }
            if (ahb2.a(n12, n3, n13).r() || n12 == n5 && n13 == n7 || n3 > n6) continue;
            n11 = this.a(ahb2, n12, n3 - 1, n13, n11);
        }
        n9 = n11 > n9 ? n11 - 1 : (n9 > 0 ? --n9 : 0);
        if (n10 > n9 - 1) {
            n9 = n10;
        }
        if (n8 != n9) {
            ahb2.a(n2, n3, n4, n9, 2);
            this.b.add(new agt(n2, n3, n4));
            this.b.add(new agt(n2 - 1, n3, n4));
            this.b.add(new agt(n2 + 1, n3, n4));
            this.b.add(new agt(n2, n3 - 1, n4));
            this.b.add(new agt(n2, n3 + 1, n4));
            this.b.add(new agt(n2, n3, n4 - 1));
            this.b.add(new agt(n2, n3, n4 + 1));
        }
    }

    private void m(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4) != this) {
            return;
        }
        ahb2.d(n2, n3, n4, this);
        ahb2.d(n2 - 1, n3, n4, this);
        ahb2.d(n2 + 1, n3, n4, this);
        ahb2.d(n2, n3, n4 - 1, this);
        ahb2.d(n2, n3, n4 + 1, this);
        ahb2.d(n2, n3 - 1, n4, this);
        ahb2.d(n2, n3 + 1, n4, this);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        if (ahb2.E) {
            return;
        }
        this.e(ahb2, n2, n3, n4);
        ahb2.d(n2, n3 + 1, n4, this);
        ahb2.d(n2, n3 - 1, n4, this);
        this.m(ahb2, n2 - 1, n3, n4);
        this.m(ahb2, n2 + 1, n3, n4);
        this.m(ahb2, n2, n3, n4 - 1);
        this.m(ahb2, n2, n3, n4 + 1);
        if (ahb2.a(n2 - 1, n3, n4).r()) {
            this.m(ahb2, n2 - 1, n3 + 1, n4);
        } else {
            this.m(ahb2, n2 - 1, n3 - 1, n4);
        }
        if (ahb2.a(n2 + 1, n3, n4).r()) {
            this.m(ahb2, n2 + 1, n3 + 1, n4);
        } else {
            this.m(ahb2, n2 + 1, n3 - 1, n4);
        }
        if (ahb2.a(n2, n3, n4 - 1).r()) {
            this.m(ahb2, n2, n3 + 1, n4 - 1);
        } else {
            this.m(ahb2, n2, n3 - 1, n4 - 1);
        }
        if (ahb2.a(n2, n3, n4 + 1).r()) {
            this.m(ahb2, n2, n3 + 1, n4 + 1);
        } else {
            this.m(ahb2, n2, n3 - 1, n4 + 1);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        super.a(ahb2, n2, n3, n4, aji2, n5);
        if (ahb2.E) {
            return;
        }
        ahb2.d(n2, n3 + 1, n4, this);
        ahb2.d(n2, n3 - 1, n4, this);
        ahb2.d(n2 + 1, n3, n4, this);
        ahb2.d(n2 - 1, n3, n4, this);
        ahb2.d(n2, n3, n4 + 1, this);
        ahb2.d(n2, n3, n4 - 1, this);
        this.e(ahb2, n2, n3, n4);
        this.m(ahb2, n2 - 1, n3, n4);
        this.m(ahb2, n2 + 1, n3, n4);
        this.m(ahb2, n2, n3, n4 - 1);
        this.m(ahb2, n2, n3, n4 + 1);
        if (ahb2.a(n2 - 1, n3, n4).r()) {
            this.m(ahb2, n2 - 1, n3 + 1, n4);
        } else {
            this.m(ahb2, n2 - 1, n3 - 1, n4);
        }
        if (ahb2.a(n2 + 1, n3, n4).r()) {
            this.m(ahb2, n2 + 1, n3 + 1, n4);
        } else {
            this.m(ahb2, n2 + 1, n3 - 1, n4);
        }
        if (ahb2.a(n2, n3, n4 - 1).r()) {
            this.m(ahb2, n2, n3 + 1, n4 - 1);
        } else {
            this.m(ahb2, n2, n3 - 1, n4 - 1);
        }
        if (ahb2.a(n2, n3, n4 + 1).r()) {
            this.m(ahb2, n2, n3 + 1, n4 + 1);
        } else {
            this.m(ahb2, n2, n3 - 1, n4 + 1);
        }
    }

    private int a(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (ahb2.a(n2, n3, n4) != this) {
            return n5;
        }
        int n6 = ahb2.e(n2, n3, n4);
        if (n6 > n5) {
            return n6;
        }
        return n5;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (ahb2.E) {
            return;
        }
        boolean bl2 = this.c(ahb2, n2, n3, n4);
        if (bl2) {
            this.e(ahb2, n2, n3, n4);
        } else {
            this.b(ahb2, n2, n3, n4, 0, 0);
            ahb2.f(n2, n3, n4);
        }
        super.a(ahb2, n2, n3, n4, aji2);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ax;
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (!this.a) {
            return 0;
        }
        return this.b(ahl2, n2, n3, n4, n5);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        boolean bl2;
        if (!this.a) {
            return 0;
        }
        int n6 = ahl2.e(n2, n3, n4);
        if (n6 == 0) {
            return 0;
        }
        if (n5 == 1) {
            return n6;
        }
        boolean bl3 = ana.g(ahl2, n2 - 1, n3, n4, 1) || !ahl2.a(n2 - 1, n3, n4).r() && ana.g(ahl2, n2 - 1, n3 - 1, n4, -1);
        boolean bl4 = ana.g(ahl2, n2 + 1, n3, n4, 3) || !ahl2.a(n2 + 1, n3, n4).r() && ana.g(ahl2, n2 + 1, n3 - 1, n4, -1);
        boolean bl5 = ana.g(ahl2, n2, n3, n4 - 1, 2) || !ahl2.a(n2, n3, n4 - 1).r() && ana.g(ahl2, n2, n3 - 1, n4 - 1, -1);
        boolean bl6 = bl2 = ana.g(ahl2, n2, n3, n4 + 1, 0) || !ahl2.a(n2, n3, n4 + 1).r() && ana.g(ahl2, n2, n3 - 1, n4 + 1, -1);
        if (!ahl2.a(n2, n3 + 1, n4).r()) {
            if (ahl2.a(n2 - 1, n3, n4).r() && ana.g(ahl2, n2 - 1, n3 + 1, n4, -1)) {
                bl3 = true;
            }
            if (ahl2.a(n2 + 1, n3, n4).r() && ana.g(ahl2, n2 + 1, n3 + 1, n4, -1)) {
                bl4 = true;
            }
            if (ahl2.a(n2, n3, n4 - 1).r() && ana.g(ahl2, n2, n3 + 1, n4 - 1, -1)) {
                bl5 = true;
            }
            if (ahl2.a(n2, n3, n4 + 1).r() && ana.g(ahl2, n2, n3 + 1, n4 + 1, -1)) {
                bl2 = true;
            }
        }
        if (!(bl5 || bl4 || bl3 || bl2 || n5 < 2 || n5 > 5)) {
            return n6;
        }
        if (n5 == 2 && bl5 && !bl3 && !bl4) {
            return n6;
        }
        if (n5 == 3 && bl2 && !bl3 && !bl4) {
            return n6;
        }
        if (n5 == 4 && bl3 && !bl5 && !bl2) {
            return n6;
        }
        if (n5 == 5 && bl4 && !bl5 && !bl2) {
            return n6;
        }
        return 0;
    }

    @Override
    public boolean f() {
        return this.a;
    }

    public static boolean f(ahl ahl2, int n2, int n3, int n4, int n5) {
        aji aji2 = ahl2.a(n2, n3, n4);
        if (aji2 == ajn.af) {
            return true;
        }
        if (ajn.aR.e(aji2)) {
            int n6 = ahl2.e(n2, n3, n4);
            return n5 == (n6 & 3) || n5 == p.f[n6 & 3];
        }
        return aji2.f() && n5 != -1;
    }

    public static boolean g(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (ana.f(ahl2, n2, n3, n4, n5)) {
            return true;
        }
        if (ahl2.a(n2, n3, n4) == ajn.aS) {
            int n6 = ahl2.e(n2, n3, n4);
            return n5 == (n6 & 3);
        }
        return false;
    }
}

