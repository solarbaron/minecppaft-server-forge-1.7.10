/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class aui {
    private static final aur[] b = new aur[]{new aur(ava.class, 40, 0), new aur(aut.class, 5, 5), new aur(aup.class, 20, 0), new aur(auu.class, 20, 0), new aur(auv.class, 10, 6), new aur(avb.class, 5, 5), new aur(auy.class, 5, 5), new aur(auo.class, 5, 4), new aur(aum.class, 5, 4), new auj(auq.class, 10, 2), new auk(aus.class, 20, 1)};
    private static List c;
    private static Class d;
    static int a;
    private static final auw e;

    public static void a() {
        avi.a(aum.class, "SHCC");
        avi.a(aun.class, "SHFC");
        avi.a(auo.class, "SH5C");
        avi.a(aup.class, "SHLT");
        avi.a(auq.class, "SHLi");
        avi.a(aus.class, "SHPR");
        avi.a(aut.class, "SHPH");
        avi.a(auu.class, "SHRT");
        avi.a(auv.class, "SHRC");
        avi.a(auy.class, "SHSD");
        avi.a(auz.class, "SHStart");
        avi.a(ava.class, "SHS");
        avi.a(avb.class, "SHSSD");
    }

    public static void b() {
        c = new ArrayList();
        for (aur aur2 : b) {
            aur2.c = 0;
            c.add(aur2);
        }
        d = null;
    }

    private static boolean d() {
        boolean bl2 = false;
        a = 0;
        for (aur aur2 : c) {
            if (aur2.d > 0 && aur2.c < aur2.d) {
                bl2 = true;
            }
            a += aur2.b;
        }
        return bl2;
    }

    private static avc a(Class clazz, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        avc avc2 = null;
        if (clazz == ava.class) {
            avc2 = ava.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == aut.class) {
            avc2 = aut.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == aup.class) {
            avc2 = aup.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == auu.class) {
            avc2 = auu.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == auv.class) {
            avc2 = auv.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avb.class) {
            avc2 = avb.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == auy.class) {
            avc2 = auy.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == auo.class) {
            avc2 = auo.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == aum.class) {
            avc2 = aum.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == auq.class) {
            avc2 = auq.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == aus.class) {
            avc2 = aus.a(list, random, n2, n3, n4, n5, n6);
        }
        return avc2;
    }

    private static avc b(auz auz2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        if (!aui.d()) {
            return null;
        }
        if (d != null) {
            avc avc2 = aui.a(d, list, random, n2, n3, n4, n5, n6);
            d = null;
            if (avc2 != null) {
                return avc2;
            }
        }
        int n7 = 0;
        block0: while (n7 < 5) {
            ++n7;
            int n8 = random.nextInt(a);
            for (aur aur2 : c) {
                if ((n8 -= aur2.b) >= 0) continue;
                if (!aur2.a(n6) || aur2 == auz2.a) continue block0;
                avc avc3 = aui.a(aur2.a, list, random, n2, n3, n4, n5, n6);
                if (avc3 == null) continue;
                ++aur2.c;
                auz2.a = aur2;
                if (!aur2.a()) {
                    c.remove(aur2);
                }
                return avc3;
            }
        }
        asv asv2 = aun.a(list, random, n2, n3, n4, n5);
        if (asv2 != null && asv2.b > 1) {
            return new aun(n6, random, asv2, n5);
        }
        return null;
    }

    private static avk c(auz auz2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        if (n6 > 50) {
            return null;
        }
        if (Math.abs(n2 - auz2.c().a) > 112 || Math.abs(n4 - auz2.c().c) > 112) {
            return null;
        }
        avc avc2 = aui.b(auz2, list, random, n2, n3, n4, n5, n6 + 1);
        if (avc2 != null) {
            list.add(avc2);
            auz2.c.add(avc2);
        }
        return avc2;
    }

    static /* synthetic */ avk a(auz auz2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        return aui.c(auz2, list, random, n2, n3, n4, n5, n6);
    }

    static /* synthetic */ Class a(Class clazz) {
        d = clazz;
        return d;
    }

    static /* synthetic */ auw c() {
        return e;
    }

    static {
        e = new auw(null);
    }
}

