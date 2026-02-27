/*
 * Decompiled with CFR 0.152.
 */
public class acn
extends adb {
    public acn() {
        this.a(abt.f);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        int n6 = ahb2.e(n2, n3, n4);
        if (yz2.a(n2, n3, n4, n5, add2) && aji2 == ajn.br && !aku.b(n6)) {
            int n7;
            int n8;
            int n9;
            if (ahb2.E) {
                return true;
            }
            ahb2.a(n2, n3, n4, n6 + 4, 2);
            ahb2.f(n2, n3, n4, ajn.br);
            --add2.b;
            for (n9 = 0; n9 < 16; ++n9) {
                double d2 = (float)n2 + (5.0f + g.nextFloat() * 6.0f) / 16.0f;
                double d3 = (float)n3 + 0.8125f;
                double d4 = (float)n4 + (5.0f + g.nextFloat() * 6.0f) / 16.0f;
                double d5 = 0.0;
                double d6 = 0.0;
                double d7 = 0.0;
                ahb2.a("smoke", d2, d3, d4, d5, d6, d7);
            }
            n9 = n6 & 3;
            int n10 = 0;
            int n11 = 0;
            boolean bl2 = false;
            boolean bl3 = true;
            int n12 = p.g[n9];
            for (n8 = -2; n8 <= 2; ++n8) {
                int n13 = n2 + p.a[n12] * n8;
                n7 = n4 + p.b[n12] * n8;
                if (ahb2.a(n13, n3, n7) != ajn.br) continue;
                if (!aku.b(ahb2.e(n13, n3, n7))) {
                    bl3 = false;
                    break;
                }
                n11 = n8;
                if (bl2) continue;
                n10 = n8;
                bl2 = true;
            }
            if (bl3 && n11 == n10 + 2) {
                for (n8 = n10; n8 <= n11; ++n8) {
                    int n14 = n2 + p.a[n12] * n8;
                    n7 = n4 + p.b[n12] * n8;
                    if (ahb2.a(n14 += p.a[n9] * 4, n3, n7 += p.b[n9] * 4) == ajn.br && aku.b(ahb2.e(n14, n3, n7))) continue;
                    bl3 = false;
                    break;
                }
                block3: for (n8 = n10 - 1; n8 <= n11 + 1; n8 += 4) {
                    for (int i2 = 1; i2 <= 3; ++i2) {
                        n7 = n2 + p.a[n12] * n8;
                        int n15 = n4 + p.b[n12] * n8;
                        if (ahb2.a(n7 += p.a[n9] * i2, n3, n15 += p.b[n9] * i2) == ajn.br && aku.b(ahb2.e(n7, n3, n15))) continue;
                        bl3 = false;
                        continue block3;
                    }
                }
                if (bl3) {
                    for (n8 = n10; n8 <= n11; ++n8) {
                        for (int i3 = 1; i3 <= 3; ++i3) {
                            n7 = n2 + p.a[n12] * n8;
                            int n16 = n4 + p.b[n12] * n8;
                            ahb2.d(n7 += p.a[n9] * i3, n3, n16 += p.b[n9] * i3, ajn.bq, 0, 2);
                        }
                    }
                }
            }
            return true;
        }
        return false;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        agt agt2;
        azu azu2 = this.a(ahb2, yz2, false);
        if (azu2 != null && azu2.a == azv.b && ahb2.a(azu2.b, azu2.c, azu2.d) == ajn.br) {
            return add2;
        }
        if (!ahb2.E && (agt2 = ahb2.b("Stronghold", (int)yz2.s, (int)yz2.t, (int)yz2.u)) != null) {
            zd zd2 = new zd(ahb2, yz2.s, yz2.t + 1.62 - (double)yz2.L, yz2.u);
            zd2.a((double)agt2.a, agt2.b, (double)agt2.c);
            ahb2.d(zd2);
            ahb2.a((sa)yz2, "random.bow", 0.5f, 0.4f / (g.nextFloat() * 0.4f + 0.8f));
            ahb2.a(null, 1002, (int)yz2.s, (int)yz2.t, (int)yz2.u, 0);
            if (!yz2.bE.d) {
                --add2.b;
            }
        }
        return add2;
    }
}

