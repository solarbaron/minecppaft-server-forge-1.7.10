/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class afv {
    private static final Random a = new Random();
    private static final agb b = new agb(null);
    private static final aga c = new aga(null);
    private static final afy d = new afy(null);
    private static final afx e = new afx(null);

    public static int a(int n2, add add2) {
        if (add2 == null) {
            return 0;
        }
        dq dq2 = add2.r();
        if (dq2 == null) {
            return 0;
        }
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            short s2 = dq2.b(i2).e("id");
            short s3 = dq2.b(i2).e("lvl");
            if (s2 != n2) continue;
            return s3;
        }
        return 0;
    }

    public static Map a(add add2) {
        dq dq2;
        LinkedHashMap<Integer, Integer> linkedHashMap = new LinkedHashMap<Integer, Integer>();
        dq dq3 = dq2 = add2.b() == ade.bR ? ade.bR.g(add2) : add2.r();
        if (dq2 != null) {
            for (int i2 = 0; i2 < dq2.c(); ++i2) {
                short s2 = dq2.b(i2).e("id");
                short s3 = dq2.b(i2).e("lvl");
                linkedHashMap.put(Integer.valueOf(s2), Integer.valueOf(s3));
            }
        }
        return linkedHashMap;
    }

    public static void a(Map map, add add2) {
        dq dq2 = new dq();
        Iterator iterator = map.keySet().iterator();
        while (iterator.hasNext()) {
            int n2 = (Integer)iterator.next();
            dh dh2 = new dh();
            dh2.a("id", (short)n2);
            dh2.a("lvl", (short)((Integer)map.get(n2)).intValue());
            dq2.a(dh2);
            if (add2.b() != ade.bR) continue;
            ade.bR.a(add2, new agc(n2, (int)((Integer)map.get(n2))));
        }
        if (dq2.c() > 0) {
            if (add2.b() != ade.bR) {
                add2.a("ench", dq2);
            }
        } else if (add2.p()) {
            add2.q().o("ench");
        }
    }

    public static int a(int n2, add[] addArray) {
        if (addArray == null) {
            return 0;
        }
        int n3 = 0;
        for (add add2 : addArray) {
            int n4 = afv.a(n2, add2);
            if (n4 <= n3) continue;
            n3 = n4;
        }
        return n3;
    }

    private static void a(afz afz2, add add2) {
        if (add2 == null) {
            return;
        }
        dq dq2 = add2.r();
        if (dq2 == null) {
            return;
        }
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            short s2 = dq2.b(i2).e("id");
            short s3 = dq2.b(i2).e("lvl");
            if (aft.b[s2] == null) continue;
            afz2.a(aft.b[s2], s3);
        }
    }

    private static void a(afz afz2, add[] addArray) {
        for (add add2 : addArray) {
            afv.a(afz2, add2);
        }
    }

    public static int a(add[] addArray, ro ro2) {
        afv.b.a = 0;
        afv.b.b = ro2;
        afv.a((afz)b, addArray);
        if (afv.b.a > 25) {
            afv.b.a = 25;
        }
        return (afv.b.a + 1 >> 1) + a.nextInt((afv.b.a >> 1) + 1);
    }

    public static float a(sv sv2, sv sv3) {
        return afv.a(sv2.be(), sv3.bd());
    }

    public static float a(add add2, sz sz2) {
        afv.c.a = 0.0f;
        afv.c.b = sz2;
        afv.a((afz)c, add2);
        return afv.c.a;
    }

    public static void a(sv sv2, sa sa2) {
        afv.d.b = sa2;
        afv.d.a = sv2;
        afv.a((afz)d, sv2.ak());
        if (sa2 instanceof yz) {
            afv.a((afz)d, sv2.be());
        }
    }

    public static void b(sv sv2, sa sa2) {
        afv.e.a = sv2;
        afv.e.b = sa2;
        afv.a((afz)e, sv2.ak());
        if (sv2 instanceof yz) {
            afv.a((afz)e, sv2.be());
        }
    }

    public static int b(sv sv2, sv sv3) {
        return afv.a(aft.o.B, sv2.be());
    }

    public static int a(sv sv2) {
        return afv.a(aft.p.B, sv2.be());
    }

    public static int b(sv sv2) {
        return afv.a(aft.i.B, sv2.ak());
    }

    public static int c(sv sv2) {
        return afv.a(aft.r.B, sv2.be());
    }

    public static boolean e(sv sv2) {
        return afv.a(aft.s.B, sv2.be()) > 0;
    }

    public static int f(sv sv2) {
        return afv.a(aft.u.B, sv2.be());
    }

    public static int g(sv sv2) {
        return afv.a(aft.z.B, sv2.be());
    }

    public static int h(sv sv2) {
        return afv.a(aft.A.B, sv2.be());
    }

    public static int i(sv sv2) {
        return afv.a(aft.q.B, sv2.be());
    }

    public static boolean j(sv sv2) {
        return afv.a(aft.j.B, sv2.ak()) > 0;
    }

    public static add a(aft aft2, sv sv2) {
        for (add add2 : sv2.ak()) {
            if (add2 == null || afv.a(aft2.B, add2) <= 0) continue;
            return add2;
        }
        return null;
    }

    public static int a(Random random, int n2, int n3, add add2) {
        adb adb2 = add2.b();
        int n4 = adb2.c();
        if (n4 <= 0) {
            return 0;
        }
        if (n3 > 15) {
            n3 = 15;
        }
        int n5 = random.nextInt(8) + 1 + (n3 >> 1) + random.nextInt(n3 + 1);
        if (n2 == 0) {
            return Math.max(n5 / 3, 1);
        }
        if (n2 == 1) {
            return n5 * 2 / 3 + 1;
        }
        return Math.max(n5, n3 * 2);
    }

    public static add a(Random random, add add2, int n2) {
        boolean bl2;
        List list = afv.b(random, add2, n2);
        boolean bl3 = bl2 = add2.b() == ade.aG;
        if (bl2) {
            add2.a(ade.bR);
        }
        if (list != null) {
            for (agc agc2 : list) {
                if (bl2) {
                    ade.bR.a(add2, agc2);
                    continue;
                }
                add2.a(agc2.b, agc2.c);
            }
        }
        return add2;
    }

    public static List b(Random random, add add2, int n2) {
        agc agc2;
        float f2;
        adb adb2 = add2.b();
        int n3 = adb2.c();
        if (n3 <= 0) {
            return null;
        }
        n3 /= 2;
        int n4 = (n3 = 1 + random.nextInt((n3 >> 1) + 1) + random.nextInt((n3 >> 1) + 1)) + n2;
        int n5 = (int)((float)n4 * (1.0f + (f2 = (random.nextFloat() + random.nextFloat() - 1.0f) * 0.15f)) + 0.5f);
        if (n5 < 1) {
            n5 = 1;
        }
        ArrayList<Object> arrayList = null;
        Map map = afv.b(n5, add2);
        if (map != null && !map.isEmpty() && (agc2 = (agc)qv.a(random, map.values())) != null) {
            arrayList = new ArrayList<Object>();
            arrayList.add(agc2);
            for (int i2 = n5; random.nextInt(50) <= i2; i2 >>= 1) {
                Object object;
                Iterator iterator = map.keySet().iterator();
                while (iterator.hasNext()) {
                    object = (Integer)iterator.next();
                    boolean bl2 = true;
                    for (agc agc3 : arrayList) {
                        if (agc3.b.a(aft.b[(Integer)object])) continue;
                        bl2 = false;
                        break;
                    }
                    if (bl2) continue;
                    iterator.remove();
                }
                if (map.isEmpty()) continue;
                object = (agc)qv.a(random, map.values());
                arrayList.add(object);
            }
        }
        return arrayList;
    }

    public static Map b(int n2, add add2) {
        adb adb2 = add2.b();
        HashMap<Integer, agc> hashMap = null;
        boolean bl2 = add2.b() == ade.aG;
        for (aft aft2 : aft.b) {
            if (aft2 == null || !aft2.C.a(adb2) && !bl2) continue;
            for (int i2 = aft2.d(); i2 <= aft2.b(); ++i2) {
                if (n2 < aft2.a(i2) || n2 > aft2.b(i2)) continue;
                if (hashMap == null) {
                    hashMap = new HashMap<Integer, agc>();
                }
                hashMap.put(aft2.B, new agc(aft2, i2));
            }
        }
        return hashMap;
    }
}

