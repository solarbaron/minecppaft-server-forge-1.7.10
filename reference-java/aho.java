/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.List;
import java.util.Random;

public final class aho {
    private HashMap a = new HashMap();

    protected static agt a(ahb ahb2, int n2, int n3) {
        apx apx2 = ahb2.e(n2, n3);
        int n4 = n2 * 16 + ahb2.s.nextInt(16);
        int n5 = n3 * 16 + ahb2.s.nextInt(16);
        int n6 = ahb2.s.nextInt(apx2 == null ? ahb2.S() : apx2.h() + 16 - 1);
        return new agt(n4, n6, n5);
    }

    public int a(mt mt2, boolean bl2, boolean bl3, boolean bl4) {
        Object object;
        Object object2;
        int n2;
        if (!bl2 && !bl3) {
            return 0;
        }
        this.a.clear();
        for (n2 = 0; n2 < mt2.h.size(); ++n2) {
            object2 = (yz)mt2.h.get(n2);
            int n3 = qh.c(((yz)object2).s / 16.0);
            int n4 = qh.c(((yz)object2).u / 16.0);
            int n5 = 8;
            for (int i2 = -n5; i2 <= n5; ++i2) {
                for (int i3 = -n5; i3 <= n5; ++i3) {
                    boolean bl5 = i2 == -n5 || i2 == n5 || i3 == -n5 || i3 == n5;
                    object = new agu(i2 + n3, i3 + n4);
                    if (!bl5) {
                        this.a.put(object, false);
                        continue;
                    }
                    if (this.a.containsKey(object)) continue;
                    this.a.put(object, true);
                }
            }
        }
        n2 = 0;
        object2 = mt2.K();
        for (sx sx2 : sx.values()) {
            if (sx2.d() && !bl3 || !sx2.d() && !bl2 || sx2.e() && !bl4 || mt2.a(sx2.a()) > sx2.b() * this.a.size() / 256) continue;
            block6: for (agu agu2 : this.a.keySet()) {
                if (((Boolean)this.a.get(agu2)).booleanValue()) continue;
                object = aho.a(mt2, agu2.a, agu2.b);
                int n6 = ((agt)object).a;
                int n7 = ((agt)object).b;
                int n8 = ((agt)object).c;
                if (mt2.a(n6, n7, n8).r() || mt2.a(n6, n7, n8).o() != sx2.c()) continue;
                int n9 = 0;
                block7: for (int i4 = 0; i4 < 3; ++i4) {
                    int n10 = n6;
                    int n11 = n7;
                    int n12 = n8;
                    int n13 = 6;
                    ahx ahx2 = null;
                    sy sy2 = null;
                    for (int i5 = 0; i5 < 4; ++i5) {
                        sw sw2;
                        float f2;
                        float f3;
                        float f4;
                        float f5;
                        float f6;
                        float f7;
                        float f8;
                        if (!aho.a(sx2, mt2, n10 += mt2.s.nextInt(n13) - mt2.s.nextInt(n13), n11 += mt2.s.nextInt(1) - mt2.s.nextInt(1), n12 += mt2.s.nextInt(n13) - mt2.s.nextInt(n13)) || mt2.a(f8 = (float)n10 + 0.5f, f7 = (float)n11, (double)(f6 = (float)n12 + 0.5f), 24.0) != null || (f5 = (f4 = f8 - (float)((r)object2).a) * f4 + (f3 = f7 - (float)((r)object2).b) * f3 + (f2 = f6 - (float)((r)object2).c) * f2) < 576.0f) continue;
                        if (ahx2 == null && (ahx2 = mt2.a(sx2, n10, n11, n12)) == null) continue block7;
                        try {
                            sw2 = (sw)ahx2.b.getConstructor(ahb.class).newInstance(mt2);
                        }
                        catch (Exception exception) {
                            exception.printStackTrace();
                            return n2;
                        }
                        sw2.b(f8, f7, f6, mt2.s.nextFloat() * 360.0f, 0.0f);
                        if (sw2.by()) {
                            mt2.d(sw2);
                            sy2 = sw2.a(sy2);
                            if (++n9 >= sw2.bB()) continue block6;
                        }
                        n2 += n9;
                    }
                }
            }
        }
        return n2;
    }

    public static boolean a(sx sx2, ahb ahb2, int n2, int n3, int n4) {
        if (sx2.c() == awt.h) {
            return ahb2.a(n2, n3, n4).o().d() && ahb2.a(n2, n3 - 1, n4).o().d() && !ahb2.a(n2, n3 + 1, n4).r();
        }
        if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3 - 1, n4);
        return aji2 != ajn.h && !ahb2.a(n2, n3, n4).r() && !ahb2.a(n2, n3, n4).o().d() && !ahb2.a(n2, n3 + 1, n4).r();
    }

    public static void a(ahb ahb2, ahu ahu2, int n2, int n3, int n4, int n5, Random random) {
        List list = ahu2.a(sx.b);
        if (list.isEmpty()) {
            return;
        }
        while (random.nextFloat() < ahu2.g()) {
            ahx ahx2 = (ahx)qv.a(ahb2.s, list);
            sy sy2 = null;
            int n6 = ahx2.c + random.nextInt(1 + ahx2.d - ahx2.c);
            int n7 = n2 + random.nextInt(n4);
            int n8 = n3 + random.nextInt(n5);
            int n9 = n7;
            int n10 = n8;
            for (int i2 = 0; i2 < n6; ++i2) {
                boolean bl2 = false;
                for (int i3 = 0; !bl2 && i3 < 4; ++i3) {
                    int n11 = ahb2.i(n7, n8);
                    if (aho.a(sx.b, ahb2, n7, n11, n8)) {
                        sw sw2;
                        float f2 = (float)n7 + 0.5f;
                        float f3 = n11;
                        float f4 = (float)n8 + 0.5f;
                        try {
                            sw2 = (sw)ahx2.b.getConstructor(ahb.class).newInstance(ahb2);
                        }
                        catch (Exception exception) {
                            exception.printStackTrace();
                            continue;
                        }
                        sw2.b(f2, f3, f4, random.nextFloat() * 360.0f, 0.0f);
                        ahb2.d(sw2);
                        sy2 = sw2.a(sy2);
                        bl2 = true;
                    }
                    n7 += random.nextInt(5) - random.nextInt(5);
                    n8 += random.nextInt(5) - random.nextInt(5);
                    while (n7 < n2 || n7 >= n2 + n4 || n8 < n3 || n8 >= n3 + n4) {
                        n7 = n9 + random.nextInt(5) - random.nextInt(5);
                        n8 = n10 + random.nextInt(5) - random.nextInt(5);
                    }
                }
            }
        }
    }
}

