/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class anc
extends aoc {
    private boolean a;
    private static Map b = new HashMap();

    private boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        if (!b.containsKey(ahb2)) {
            b.put(ahb2, new ArrayList());
        }
        List list = (List)b.get(ahb2);
        if (bl2) {
            list.add(new and(n2, n3, n4, ahb2.I()));
        }
        int n5 = 0;
        for (int i2 = 0; i2 < list.size(); ++i2) {
            and and2 = (and)list.get(i2);
            if (and2.a != n2 || and2.b != n3 || and2.c != n4 || ++n5 < 8) continue;
            return true;
        }
        return false;
    }

    protected anc(boolean bl2) {
        this.a = bl2;
        this.a(true);
        this.a((abt)null);
    }

    @Override
    public int a(ahb ahb2) {
        return 2;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.e(n2, n3, n4) == 0) {
            super.b(ahb2, n2, n3, n4);
        }
        if (this.a) {
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.d(n2, n3 + 1, n4, this);
            ahb2.d(n2 - 1, n3, n4, this);
            ahb2.d(n2 + 1, n3, n4, this);
            ahb2.d(n2, n3, n4 - 1, this);
            ahb2.d(n2, n3, n4 + 1, this);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        if (this.a) {
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.d(n2, n3 + 1, n4, this);
            ahb2.d(n2 - 1, n3, n4, this);
            ahb2.d(n2 + 1, n3, n4, this);
            ahb2.d(n2, n3, n4 - 1, this);
            ahb2.d(n2, n3, n4 + 1, this);
        }
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (!this.a) {
            return 0;
        }
        int n6 = ahl2.e(n2, n3, n4);
        if (n6 == 5 && n5 == 1) {
            return 0;
        }
        if (n6 == 3 && n5 == 3) {
            return 0;
        }
        if (n6 == 4 && n5 == 2) {
            return 0;
        }
        if (n6 == 1 && n5 == 5) {
            return 0;
        }
        if (n6 == 2 && n5 == 4) {
            return 0;
        }
        return 15;
    }

    private boolean m(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        if (n5 == 5 && ahb2.f(n2, n3 - 1, n4, 0)) {
            return true;
        }
        if (n5 == 3 && ahb2.f(n2, n3, n4 - 1, 2)) {
            return true;
        }
        if (n5 == 4 && ahb2.f(n2, n3, n4 + 1, 3)) {
            return true;
        }
        if (n5 == 1 && ahb2.f(n2 - 1, n3, n4, 4)) {
            return true;
        }
        return n5 == 2 && ahb2.f(n2 + 1, n3, n4, 5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        boolean bl2 = this.m(ahb2, n2, n3, n4);
        List list = (List)b.get(ahb2);
        while (list != null && !list.isEmpty() && ahb2.I() - ((and)list.get((int)0)).d > 60L) {
            list.remove(0);
        }
        if (this.a) {
            if (bl2) {
                ahb2.d(n2, n3, n4, ajn.az, ahb2.e(n2, n3, n4), 3);
                if (this.a(ahb2, n2, n3, n4, true)) {
                    ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), "random.fizz", 0.5f, 2.6f + (ahb2.s.nextFloat() - ahb2.s.nextFloat()) * 0.8f);
                    for (int i2 = 0; i2 < 5; ++i2) {
                        double d2 = (double)n2 + random.nextDouble() * 0.6 + 0.2;
                        double d3 = (double)n3 + random.nextDouble() * 0.6 + 0.2;
                        double d4 = (double)n4 + random.nextDouble() * 0.6 + 0.2;
                        ahb2.a("smoke", d2, d3, d4, 0.0, 0.0, 0.0);
                    }
                }
            }
        } else if (!bl2 && !this.a(ahb2, n2, n3, n4, false)) {
            ahb2.d(n2, n3, n4, ajn.aA, ahb2.e(n2, n3, n4), 3);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (this.b(ahb2, n2, n3, n4, aji2)) {
            return;
        }
        boolean bl2 = this.m(ahb2, n2, n3, n4);
        if (this.a && bl2 || !this.a && !bl2) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
        }
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (n5 == 0) {
            return this.b(ahl2, n2, n3, n4, n5);
        }
        return 0;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.aA);
    }

    @Override
    public boolean f() {
        return true;
    }

    @Override
    public boolean c(aji aji2) {
        return aji2 == ajn.az || aji2 == ajn.aA;
    }
}

