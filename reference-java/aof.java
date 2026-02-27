/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aof
extends aji {
    public aof() {
        super(awt.q);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.15625f, 1.0f);
        this.a(true);
    }

    @Override
    public int a(ahb ahb2) {
        return 10;
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
        return 30;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.F;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        boolean bl2;
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl3 = (n5 & 2) == 2;
        boolean bl4 = bl2 = !ahb.a(ahb2, n2, n3 - 1, n4);
        if (bl3 != bl2) {
            this.b(ahb2, n2, n3, n4, n5, 0);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        boolean bl2;
        int n5 = ahl2.e(n2, n3, n4);
        boolean bl3 = (n5 & 4) == 4;
        boolean bl4 = bl2 = (n5 & 2) == 2;
        if (!bl2) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.09375f, 1.0f);
        } else if (!bl3) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        } else {
            this.a(0.0f, 0.0625f, 0.0f, 1.0f, 0.15625f, 1.0f);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb.a(ahb2, n2, n3 - 1, n4) ? 0 : 2;
        ahb2.a(n2, n3, n4, n5, 3);
        this.a(ahb2, n2, n3, n4, n5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        this.a(ahb2, n2, n3, n4, n5 | 1);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        if (ahb2.E) {
            return;
        }
        if (yz2.bF() != null && yz2.bF().b() == ade.aZ) {
            ahb2.a(n2, n3, n4, n5 | 8, 4);
        }
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        block0: for (int i2 = 0; i2 < 2; ++i2) {
            for (int i3 = 1; i3 < 42; ++i3) {
                int n6 = n2 + p.a[i2] * i3;
                int n7 = n4 + p.b[i2] * i3;
                aji aji2 = ahb2.a(n6, n3, n7);
                if (aji2 == ajn.bC) {
                    int n8 = ahb2.e(n6, n3, n7) & 3;
                    if (n8 != p.f[i2]) continue block0;
                    ajn.bC.a(ahb2, n6, n3, n7, false, ahb2.e(n6, n3, n7), true, i3, n5);
                    continue block0;
                }
                if (aji2 != ajn.bD) continue block0;
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        if (ahb2.E) {
            return;
        }
        if ((ahb2.e(n2, n3, n4) & 1) == 1) {
            return;
        }
        this.e(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        if ((ahb2.e(n2, n3, n4) & 1) != 1) {
            return;
        }
        this.e(ahb2, n2, n3, n4);
    }

    private void e(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl2 = (n5 & 1) == 1;
        boolean bl3 = false;
        List list = ahb2.b(null, azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (double)n3 + this.F, (double)n4 + this.G));
        if (!list.isEmpty()) {
            for (sa sa2 : list) {
                if (sa2.az()) continue;
                bl3 = true;
                break;
            }
        }
        if (bl3 && !bl2) {
            n5 |= 1;
        }
        if (!bl3 && bl2) {
            n5 &= 0xFFFFFFFE;
        }
        if (bl3 != bl2) {
            ahb2.a(n2, n3, n4, n5, 3);
            this.a(ahb2, n2, n3, n4, n5);
        }
        if (bl3) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
        }
    }
}

