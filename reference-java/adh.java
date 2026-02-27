/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.HashMultiset;
import com.google.common.collect.Iterables;
import com.google.common.collect.Multisets;

public class adh
extends abs {
    protected adh() {
        this.a(true);
    }

    public ayi a(add add2, ahb ahb2) {
        String string = "map_" + add2.k();
        ayi ayi2 = (ayi)ahb2.a(ayi.class, string);
        if (ayi2 == null && !ahb2.E) {
            add2.b(ahb2.b("map"));
            string = "map_" + add2.k();
            ayi2 = new ayi(string);
            ayi2.d = (byte)3;
            int n2 = 128 * (1 << ayi2.d);
            ayi2.a = Math.round((float)ahb2.N().c() / (float)n2) * n2;
            ayi2.b = Math.round(ahb2.N().e() / n2) * n2;
            ayi2.c = (byte)ahb2.t.i;
            ayi2.c();
            ahb2.a(string, ayi2);
        }
        return ayi2;
    }

    public void a(ahb ahb2, sa sa2, ayi ayi2) {
        if (ahb2.t.i != ayi2.c || !(sa2 instanceof yz)) {
            return;
        }
        int n2 = 1 << ayi2.d;
        int n3 = ayi2.a;
        int n4 = ayi2.b;
        int n5 = qh.c(sa2.s - (double)n3) / n2 + 64;
        int n6 = qh.c(sa2.u - (double)n4) / n2 + 64;
        int n7 = 128 / n2;
        if (ahb2.t.g) {
            n7 /= 2;
        }
        ayj ayj2 = ayi2.a((yz)sa2);
        ++ayj2.d;
        for (int i2 = n5 - n7 + 1; i2 < n5 + n7; ++i2) {
            if ((i2 & 0xF) != (ayj2.d & 0xF)) continue;
            int n8 = 255;
            int n9 = 0;
            double d2 = 0.0;
            for (int i3 = n6 - n7 - 1; i3 < n6 + n7; ++i3) {
                int n10;
                int n11;
                Object object;
                int n12;
                int n13;
                if (i2 < 0 || i3 < -1 || i2 >= 128 || i3 >= 128) continue;
                int n14 = i2 - n5;
                int n15 = i3 - n6;
                boolean bl2 = n14 * n14 + n15 * n15 > (n7 - 2) * (n7 - 2);
                int n16 = (n3 / n2 + i2 - 64) * n2;
                int n17 = (n4 / n2 + i3 - 64) * n2;
                HashMultiset<awv> hashMultiset = HashMultiset.create();
                apx apx2 = ahb2.d(n16, n17);
                if (apx2.g()) continue;
                int n18 = n16 & 0xF;
                int n19 = n17 & 0xF;
                int n20 = 0;
                double d3 = 0.0;
                if (ahb2.t.g) {
                    n13 = n16 + n17 * 231871;
                    if (((n13 = n13 * n13 * 31287121 + n13 * 11) >> 20 & 1) == 0) {
                        hashMultiset.add(ajn.d.f(0), 10);
                    } else {
                        hashMultiset.add(ajn.b.f(0), 100);
                    }
                    d3 = 100.0;
                } else {
                    for (n13 = 0; n13 < n2; ++n13) {
                        for (int i4 = 0; i4 < n2; ++i4) {
                            n12 = apx2.b(n13 + n18, i4 + n19) + 1;
                            object = ajn.a;
                            n11 = 0;
                            if (n12 > 1) {
                                while (((aji)(object = apx2.a(n13 + n18, --n12, i4 + n19))).f(n11 = apx2.c(n13 + n18, n12, i4 + n19)) == awv.b && n12 > 0) {
                                }
                                if (n12 > 0 && ((aji)object).o().d()) {
                                    aji aji2;
                                    n10 = n12 - 1;
                                    do {
                                        aji2 = apx2.a(n13 + n18, n10--, i4 + n19);
                                        ++n20;
                                    } while (n10 > 0 && aji2.o().d());
                                }
                            }
                            d3 += (double)n12 / (double)(n2 * n2);
                            hashMultiset.add(((aji)object).f(n11));
                        }
                    }
                }
                n20 /= n2 * n2;
                double d4 = (d3 - d2) * 4.0 / (double)(n2 + 4) + ((double)(i2 + i3 & 1) - 0.5) * 0.4;
                n12 = 1;
                if (d4 > 0.6) {
                    n12 = 2;
                }
                if (d4 < -0.6) {
                    n12 = 0;
                }
                if ((object = Iterables.getFirst(Multisets.copyHighestCountFirst(hashMultiset), awv.b)) == awv.n) {
                    d4 = (double)n20 * 0.1 + (double)(i2 + i3 & 1) * 0.2;
                    n12 = 1;
                    if (d4 < 0.5) {
                        n12 = 2;
                    }
                    if (d4 > 0.9) {
                        n12 = 0;
                    }
                }
                d2 = d3;
                if (i3 < 0 || n14 * n14 + n15 * n15 >= n7 * n7 || bl2 && (i2 + i3 & 1) == 0 || (n11 = ayi2.e[i2 + i3 * 128]) == (n10 = (int)((byte)(((awv)object).M * 4 + n12)))) continue;
                if (n8 > i3) {
                    n8 = i3;
                }
                if (n9 < i3) {
                    n9 = i3;
                }
                ayi2.e[i2 + i3 * 128] = n10;
            }
            if (n8 > n9) continue;
            ayi2.a(i2, n8, n9);
        }
    }

    @Override
    public void a(add add2, ahb ahb2, sa sa2, int n2, boolean bl2) {
        if (ahb2.E) {
            return;
        }
        ayi ayi2 = this.a(add2, ahb2);
        if (sa2 instanceof yz) {
            yz yz2 = (yz)sa2;
            ayi2.a(yz2, add2);
        }
        if (bl2) {
            this.a(ahb2, sa2, ayi2);
        }
    }

    @Override
    public ft c(add add2, ahb ahb2, yz yz2) {
        byte[] byArray = this.a(add2, ahb2).a(add2, ahb2, yz2);
        if (byArray == null) {
            return null;
        }
        return new he(add2.k(), byArray);
    }

    @Override
    public void d(add add2, ahb ahb2, yz yz2) {
        if (add2.p() && add2.q().n("map_is_scaling")) {
            ayi ayi2 = ade.aY.a(add2, ahb2);
            add2.b(ahb2.b("map"));
            ayi ayi3 = new ayi("map_" + add2.k());
            ayi3.d = (byte)(ayi2.d + 1);
            if (ayi3.d > 4) {
                ayi3.d = (byte)4;
            }
            ayi3.a = ayi2.a;
            ayi3.b = ayi2.b;
            ayi3.c = ayi2.c;
            ayi3.c();
            ahb2.a("map_" + add2.k(), ayi3);
        }
    }
}

