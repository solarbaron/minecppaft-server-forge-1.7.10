/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class asx {
    private static final qx[] a = new qx[]{new qx(ade.j, 0, 1, 5, 10), new qx(ade.k, 0, 1, 3, 5), new qx(ade.ax, 0, 4, 9, 5), new qx(ade.aR, 4, 4, 9, 5), new qx(ade.i, 0, 1, 2, 3), new qx(ade.h, 0, 3, 8, 10), new qx(ade.P, 0, 1, 3, 15), new qx(ade.b, 0, 1, 1, 1), new qx(adb.a(ajn.aq), 0, 4, 8, 1), new qx(ade.bc, 0, 2, 4, 10), new qx(ade.bb, 0, 2, 4, 10), new qx(ade.av, 0, 1, 1, 3), new qx(ade.bX, 0, 1, 1, 1)};

    public static void a() {
        avi.a(asy.class, "MSCorridor");
        avi.a(asz.class, "MSCrossing");
        avi.a(ata.class, "MSRoom");
        avi.a(atb.class, "MSStairs");
    }

    private static avk a(List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        int n7 = random.nextInt(100);
        if (n7 >= 80) {
            asv asv2 = asz.a(list, random, n2, n3, n4, n5);
            if (asv2 != null) {
                return new asz(n6, random, asv2, n5);
            }
        } else if (n7 >= 70) {
            asv asv3 = atb.a(list, random, n2, n3, n4, n5);
            if (asv3 != null) {
                return new atb(n6, random, asv3, n5);
            }
        } else {
            asv asv4 = asy.a(list, random, n2, n3, n4, n5);
            if (asv4 != null) {
                return new asy(n6, random, asv4, n5);
            }
        }
        return null;
    }

    private static avk b(avk avk2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        if (n6 > 8) {
            return null;
        }
        if (Math.abs(n2 - avk2.c().a) > 80 || Math.abs(n4 - avk2.c().c) > 80) {
            return null;
        }
        avk avk3 = asx.a(list, random, n2, n3, n4, n5, n6 + 1);
        if (avk3 != null) {
            list.add(avk3);
            avk3.a(avk2, list, random);
        }
        return avk3;
    }

    static /* synthetic */ avk a(avk avk2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        return asx.b(avk2, list, random, n2, n3, n4, n5, n6);
    }

    static /* synthetic */ qx[] b() {
        return a;
    }
}

