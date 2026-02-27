/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class asd
extends arn {
    private static final qx[] a = new qx[]{new qx(ade.av, 0, 1, 1, 10), new qx(ade.j, 0, 1, 4, 10), new qx(ade.P, 0, 1, 1, 10), new qx(ade.O, 0, 1, 4, 10), new qx(ade.H, 0, 1, 4, 10), new qx(ade.F, 0, 1, 4, 10), new qx(ade.ar, 0, 1, 1, 10), new qx(ade.ao, 0, 1, 1, 1), new qx(ade.ax, 0, 1, 4, 10), new qx(ade.cd, 0, 1, 1, 10), new qx(ade.ce, 0, 1, 1, 10), new qx(ade.cb, 0, 1, 1, 10), new qx(ade.bY, 0, 1, 1, 2), new qx(ade.bX, 0, 1, 1, 5), new qx(ade.bZ, 0, 1, 1, 1)};

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5;
        int n6;
        int n7;
        int n8 = 3;
        int n9 = random.nextInt(2) + 2;
        int n10 = random.nextInt(2) + 2;
        int n11 = 0;
        for (n7 = n2 - n9 - 1; n7 <= n2 + n9 + 1; ++n7) {
            for (n6 = n3 - 1; n6 <= n3 + n8 + 1; ++n6) {
                for (n5 = n4 - n10 - 1; n5 <= n4 + n10 + 1; ++n5) {
                    awt awt2 = ahb2.a(n7, n6, n5).o();
                    if (n6 == n3 - 1 && !awt2.a()) {
                        return false;
                    }
                    if (n6 == n3 + n8 + 1 && !awt2.a()) {
                        return false;
                    }
                    if (n7 != n2 - n9 - 1 && n7 != n2 + n9 + 1 && n5 != n4 - n10 - 1 && n5 != n4 + n10 + 1 || n6 != n3 || !ahb2.c(n7, n6, n5) || !ahb2.c(n7, n6 + 1, n5)) continue;
                    ++n11;
                }
            }
        }
        if (n11 < 1 || n11 > 5) {
            return false;
        }
        for (n7 = n2 - n9 - 1; n7 <= n2 + n9 + 1; ++n7) {
            for (n6 = n3 + n8; n6 >= n3 - 1; --n6) {
                for (n5 = n4 - n10 - 1; n5 <= n4 + n10 + 1; ++n5) {
                    if (n7 == n2 - n9 - 1 || n6 == n3 - 1 || n5 == n4 - n10 - 1 || n7 == n2 + n9 + 1 || n6 == n3 + n8 + 1 || n5 == n4 + n10 + 1) {
                        if (n6 >= 0 && !ahb2.a(n7, n6 - 1, n5).o().a()) {
                            ahb2.f(n7, n6, n5);
                            continue;
                        }
                        if (!ahb2.a(n7, n6, n5).o().a()) continue;
                        if (n6 == n3 - 1 && random.nextInt(4) != 0) {
                            ahb2.d(n7, n6, n5, ajn.Y, 0, 2);
                            continue;
                        }
                        ahb2.d(n7, n6, n5, ajn.e, 0, 2);
                        continue;
                    }
                    ahb2.f(n7, n6, n5);
                }
            }
        }
        block6: for (n7 = 0; n7 < 2; ++n7) {
            for (n6 = 0; n6 < 3; ++n6) {
                int n12;
                int n13;
                n5 = n2 + random.nextInt(n9 * 2 + 1) - n9;
                if (!ahb2.c(n5, n13 = n3, n12 = n4 + random.nextInt(n10 * 2 + 1) - n10)) continue;
                int n14 = 0;
                if (ahb2.a(n5 - 1, n13, n12).o().a()) {
                    ++n14;
                }
                if (ahb2.a(n5 + 1, n13, n12).o().a()) {
                    ++n14;
                }
                if (ahb2.a(n5, n13, n12 - 1).o().a()) {
                    ++n14;
                }
                if (ahb2.a(n5, n13, n12 + 1).o().a()) {
                    ++n14;
                }
                if (n14 != 1) continue;
                ahb2.d(n5, n13, n12, ajn.ae, 0, 2);
                qx[] qxArray = qx.a(a, ade.bR.b(random));
                aow aow2 = (aow)ahb2.o(n5, n13, n12);
                if (aow2 == null) continue block6;
                qx.a(random, qxArray, aow2, 8);
                continue block6;
            }
        }
        ahb2.d(n2, n3, n4, ajn.ac, 0, 2);
        apj apj2 = (apj)ahb2.o(n2, n3, n4);
        if (apj2 != null) {
            apj2.a().a(this.a(random));
        } else {
            System.err.println("Failed to fetch mob spawner entity at (" + n2 + ", " + n3 + ", " + n4 + ")");
        }
        return true;
    }

    private String a(Random random) {
        int n2 = random.nextInt(4);
        if (n2 == 0) {
            return "Skeleton";
        }
        if (n2 == 1) {
            return "Zombie";
        }
        if (n2 == 2) {
            return "Zombie";
        }
        if (n2 == 3) {
            return "Spider";
        }
        return "";
    }
}

