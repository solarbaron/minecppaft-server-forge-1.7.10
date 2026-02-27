/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

public class avp {
    public static void a() {
        avi.a(avq.class, "ViBH");
        avi.a(avr.class, "ViDF");
        avi.a(avs.class, "ViF");
        avi.a(avt.class, "ViL");
        avi.a(avv.class, "ViPH");
        avi.a(avw.class, "ViSH");
        avi.a(avx.class, "ViSmH");
        avi.a(avy.class, "ViST");
        avi.a(avz.class, "ViS");
        avi.a(awa.class, "ViStart");
        avi.a(awb.class, "ViSR");
        avi.a(awc.class, "ViTRH");
        avi.a(awf.class, "ViW");
    }

    public static List a(Random random, int n2) {
        ArrayList<avu> arrayList = new ArrayList<avu>();
        arrayList.add(new avu(avw.class, 4, qh.a(random, 2 + n2, 4 + n2 * 2)));
        arrayList.add(new avu(avy.class, 20, qh.a(random, 0 + n2, 1 + n2)));
        arrayList.add(new avu(avq.class, 20, qh.a(random, 0 + n2, 2 + n2)));
        arrayList.add(new avu(avx.class, 3, qh.a(random, 2 + n2, 5 + n2 * 3)));
        arrayList.add(new avu(avv.class, 15, qh.a(random, 0 + n2, 2 + n2)));
        arrayList.add(new avu(avr.class, 3, qh.a(random, 1 + n2, 4 + n2)));
        arrayList.add(new avu(avs.class, 3, qh.a(random, 2 + n2, 4 + n2 * 2)));
        arrayList.add(new avu(avz.class, 15, qh.a(random, 0, 1 + n2)));
        arrayList.add(new avu(awc.class, 8, qh.a(random, 0 + n2, 3 + n2 * 2)));
        Iterator iterator = arrayList.iterator();
        while (iterator.hasNext()) {
            if (((avu)iterator.next()).d != 0) continue;
            iterator.remove();
        }
        return arrayList;
    }

    private static int a(List list) {
        boolean bl2 = false;
        int n2 = 0;
        for (avu avu2 : list) {
            if (avu2.d > 0 && avu2.c < avu2.d) {
                bl2 = true;
            }
            n2 += avu2.b;
        }
        return bl2 ? n2 : -1;
    }

    private static awd a(awa awa2, avu avu2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        Class clazz = avu2.a;
        awd awd2 = null;
        if (clazz == avw.class) {
            awd2 = avw.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avy.class) {
            awd2 = avy.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avq.class) {
            awd2 = avq.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avx.class) {
            awd2 = avx.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avv.class) {
            awd2 = avv.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avr.class) {
            awd2 = avr.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avs.class) {
            awd2 = avs.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == avz.class) {
            awd2 = avz.a(awa2, list, random, n2, n3, n4, n5, n6);
        } else if (clazz == awc.class) {
            awd2 = awc.a(awa2, list, random, n2, n3, n4, n5, n6);
        }
        return awd2;
    }

    private static awd c(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        int n7 = avp.a(awa2.e);
        if (n7 <= 0) {
            return null;
        }
        int n8 = 0;
        block0: while (n8 < 5) {
            ++n8;
            int n9 = random.nextInt(n7);
            for (avu avu2 : awa2.e) {
                if ((n9 -= avu2.b) >= 0) continue;
                if (!avu2.a(n6) || avu2 == awa2.d && awa2.e.size() > 1) continue block0;
                awd awd2 = avp.a(awa2, avu2, list, random, n2, n3, n4, n5, n6);
                if (awd2 == null) continue;
                ++avu2.c;
                awa2.d = avu2;
                if (!avu2.a()) {
                    awa2.e.remove(avu2);
                }
                return awd2;
            }
        }
        asv asv2 = avt.a(awa2, list, random, n2, n3, n4, n5);
        if (asv2 != null) {
            return new avt(awa2, n6, random, asv2, n5);
        }
        return null;
    }

    private static avk d(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        if (n6 > 50) {
            return null;
        }
        if (Math.abs(n2 - awa2.c().a) > 112 || Math.abs(n4 - awa2.c().c) > 112) {
            return null;
        }
        awd awd2 = avp.c(awa2, list, random, n2, n3, n4, n5, n6 + 1);
        if (awd2 != null) {
            int n7;
            int n8 = (awd2.f.a + awd2.f.d) / 2;
            int n9 = (awd2.f.c + awd2.f.f) / 2;
            int n10 = awd2.f.d - awd2.f.a;
            int n11 = awd2.f.f - awd2.f.c;
            int n12 = n7 = n10 > n11 ? n10 : n11;
            if (awa2.e().a(n8, n9, n7 / 2 + 4, avn.e)) {
                list.add(awd2);
                awa2.i.add(awd2);
                return awd2;
            }
        }
        return null;
    }

    private static avk e(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        if (n6 > 3 + awa2.c) {
            return null;
        }
        if (Math.abs(n2 - awa2.c().a) > 112 || Math.abs(n4 - awa2.c().c) > 112) {
            return null;
        }
        asv asv2 = awb.a(awa2, list, random, n2, n3, n4, n5);
        if (asv2 != null && asv2.b > 10) {
            int n7;
            awb awb2 = new awb(awa2, n6, random, asv2, n5);
            int n8 = (awb2.f.a + awb2.f.d) / 2;
            int n9 = (awb2.f.c + awb2.f.f) / 2;
            int n10 = awb2.f.d - awb2.f.a;
            int n11 = awb2.f.f - awb2.f.c;
            int n12 = n7 = n10 > n11 ? n10 : n11;
            if (awa2.e().a(n8, n9, n7 / 2 + 4, avn.e)) {
                list.add(awb2);
                awa2.j.add(awb2);
                return awb2;
            }
        }
        return null;
    }

    static /* synthetic */ avk a(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        return avp.d(awa2, list, random, n2, n3, n4, n5, n6);
    }

    static /* synthetic */ avk b(awa awa2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        return avp.e(awa2, list, random, n2, n3, n4, n5, n6);
    }
}

