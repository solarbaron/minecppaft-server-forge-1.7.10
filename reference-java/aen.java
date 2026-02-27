/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

public class aen {
    public static final String a;
    public static final String b;
    public static final String c;
    public static final String d;
    public static final String e;
    public static final String f;
    public static final String g;
    public static final String h;
    public static final String i;
    public static final String j;
    public static final String k;
    public static final String l;
    public static final String m;
    private static final HashMap n;
    private static final HashMap o;
    private static final HashMap p;
    private static final String[] q;

    public static boolean a(int n2, int n3) {
        return (n2 & 1 << n3) != 0;
    }

    private static int c(int n2, int n3) {
        return aen.a(n2, n3) ? 1 : 0;
    }

    private static int d(int n2, int n3) {
        return aen.a(n2, n3) ? 0 : 1;
    }

    public static int a(int n2) {
        return aen.a(n2, 5, 4, 3, 2, 1);
    }

    public static int a(Collection collection) {
        int n2 = 3694022;
        if (collection == null || collection.isEmpty()) {
            return n2;
        }
        float f2 = 0.0f;
        float f3 = 0.0f;
        float f4 = 0.0f;
        float f5 = 0.0f;
        for (rw rw2 : collection) {
            int n3 = rv.a[rw2.a()].j();
            for (int i2 = 0; i2 <= rw2.c(); ++i2) {
                f2 += (float)(n3 >> 16 & 0xFF) / 255.0f;
                f3 += (float)(n3 >> 8 & 0xFF) / 255.0f;
                f4 += (float)(n3 >> 0 & 0xFF) / 255.0f;
                f5 += 1.0f;
            }
        }
        f2 = f2 / f5 * 255.0f;
        f3 = f3 / f5 * 255.0f;
        f4 = f4 / f5 * 255.0f;
        return (int)f2 << 16 | (int)f3 << 8 | (int)f4;
    }

    public static boolean b(Collection collection) {
        for (rw rw2 : collection) {
            if (rw2.e()) continue;
            return false;
        }
        return true;
    }

    public static String c(int n2) {
        int n3 = aen.a(n2);
        return q[n3];
    }

    private static int a(boolean bl2, boolean bl3, boolean bl4, int n2, int n3, int n4, int n5) {
        int n6 = 0;
        if (bl2) {
            n6 = aen.d(n5, n3);
        } else if (n2 != -1) {
            if (n2 == 0 && aen.h(n5) == n3) {
                n6 = 1;
            } else if (n2 == 1 && aen.h(n5) > n3) {
                n6 = 1;
            } else if (n2 == 2 && aen.h(n5) < n3) {
                n6 = 1;
            }
        } else {
            n6 = aen.c(n5, n3);
        }
        if (bl3) {
            n6 *= n4;
        }
        if (bl4) {
            n6 *= -1;
        }
        return n6;
    }

    private static int h(int n2) {
        int n3 = 0;
        while (n2 > 0) {
            n2 &= n2 - 1;
            ++n3;
        }
        return n3;
    }

    private static int a(String string, int n2, int n3, int n4) {
        if (n2 >= string.length() || n3 < 0 || n2 >= n3) {
            return 0;
        }
        int n5 = string.indexOf(124, n2);
        if (n5 >= 0 && n5 < n3) {
            int n6 = aen.a(string, n2, n5 - 1, n4);
            if (n6 > 0) {
                return n6;
            }
            int n7 = aen.a(string, n5 + 1, n3, n4);
            if (n7 > 0) {
                return n7;
            }
            return 0;
        }
        int n8 = string.indexOf(38, n2);
        if (n8 >= 0 && n8 < n3) {
            int n9 = aen.a(string, n2, n8 - 1, n4);
            if (n9 <= 0) {
                return 0;
            }
            int n10 = aen.a(string, n8 + 1, n3, n4);
            if (n10 <= 0) {
                return 0;
            }
            if (n9 > n10) {
                return n9;
            }
            return n10;
        }
        boolean bl2 = false;
        boolean bl3 = false;
        boolean bl4 = false;
        boolean bl5 = false;
        boolean bl6 = false;
        int n11 = -1;
        int n12 = 0;
        int n13 = 0;
        int n14 = 0;
        for (int i2 = n2; i2 < n3; ++i2) {
            char c2 = string.charAt(i2);
            if (c2 >= '0' && c2 <= '9') {
                if (bl2) {
                    n13 = c2 - 48;
                    bl3 = true;
                    continue;
                }
                n12 *= 10;
                n12 += c2 - 48;
                bl4 = true;
                continue;
            }
            if (c2 == '*') {
                bl2 = true;
                continue;
            }
            if (c2 == '!') {
                if (bl4) {
                    n14 += aen.a(bl5, bl3, bl6, n11, n12, n13, n4);
                    bl5 = false;
                    bl6 = false;
                    bl2 = false;
                    bl3 = false;
                    bl4 = false;
                    n13 = 0;
                    n12 = 0;
                    n11 = -1;
                }
                bl5 = true;
                continue;
            }
            if (c2 == '-') {
                if (bl4) {
                    n14 += aen.a(bl5, bl3, bl6, n11, n12, n13, n4);
                    bl5 = false;
                    bl6 = false;
                    bl2 = false;
                    bl3 = false;
                    bl4 = false;
                    n13 = 0;
                    n12 = 0;
                    n11 = -1;
                }
                bl6 = true;
                continue;
            }
            if (c2 == '=' || c2 == '<' || c2 == '>') {
                if (bl4) {
                    n14 += aen.a(bl5, bl3, bl6, n11, n12, n13, n4);
                    bl5 = false;
                    bl6 = false;
                    bl2 = false;
                    bl3 = false;
                    bl4 = false;
                    n13 = 0;
                    n12 = 0;
                    n11 = -1;
                }
                if (c2 == '=') {
                    n11 = 0;
                    continue;
                }
                if (c2 == '<') {
                    n11 = 2;
                    continue;
                }
                if (c2 != '>') continue;
                n11 = 1;
                continue;
            }
            if (c2 != '+' || !bl4) continue;
            n14 += aen.a(bl5, bl3, bl6, n11, n12, n13, n4);
            bl5 = false;
            bl6 = false;
            bl2 = false;
            bl3 = false;
            bl4 = false;
            n13 = 0;
            n12 = 0;
            n11 = -1;
        }
        if (bl4) {
            n14 += aen.a(bl5, bl3, bl6, n11, n12, n13, n4);
        }
        return n14;
    }

    public static List b(int n2, boolean bl2) {
        ArrayList<rw> arrayList = null;
        for (rv rv2 : rv.a) {
            int n3;
            String string;
            if (rv2 == null || rv2.i() && !bl2 || (string = (String)n.get(rv2.c())) == null || (n3 = aen.a(string, 0, string.length(), n2)) <= 0) continue;
            int n4 = 0;
            String string2 = (String)o.get(rv2.c());
            if (string2 != null && (n4 = aen.a(string2, 0, string2.length(), n2)) < 0) {
                n4 = 0;
            }
            if (rv2.b()) {
                n3 = 1;
            } else {
                n3 = 1200 * (n3 * 3 + (n3 - 1) * 2);
                n3 >>= n4;
                n3 = (int)Math.round((double)n3 * rv2.g());
                if ((n2 & 0x4000) != 0) {
                    n3 = (int)Math.round((double)n3 * 0.75 + 0.5);
                }
            }
            if (arrayList == null) {
                arrayList = new ArrayList<rw>();
            }
            rw rw2 = new rw(rv2.c(), n3, n4);
            if ((n2 & 0x4000) != 0) {
                rw2.a(true);
            }
            arrayList.add(rw2);
        }
        return arrayList;
    }

    private static int a(int n2, int n3, boolean bl2, boolean bl3, boolean bl4) {
        if (bl4) {
            if (!aen.a(n2, n3)) {
                return 0;
            }
        } else {
            n2 = bl2 ? (n2 &= ~(1 << n3)) : (bl3 ? ((n2 & 1 << n3) == 0 ? (n2 |= 1 << n3) : (n2 &= ~(1 << n3))) : (n2 |= 1 << n3));
        }
        return n2;
    }

    public static int a(int n2, String string) {
        int n3 = 0;
        int n4 = string.length();
        boolean bl2 = false;
        boolean bl3 = false;
        boolean bl4 = false;
        boolean bl5 = false;
        int n5 = 0;
        for (int i2 = n3; i2 < n4; ++i2) {
            char c2 = string.charAt(i2);
            if (c2 >= '0' && c2 <= '9') {
                n5 *= 10;
                n5 += c2 - 48;
                bl2 = true;
                continue;
            }
            if (c2 == '!') {
                if (bl2) {
                    n2 = aen.a(n2, n5, bl4, bl3, bl5);
                    bl5 = false;
                    bl3 = false;
                    bl4 = false;
                    bl2 = false;
                    n5 = 0;
                }
                bl3 = true;
                continue;
            }
            if (c2 == '-') {
                if (bl2) {
                    n2 = aen.a(n2, n5, bl4, bl3, bl5);
                    bl5 = false;
                    bl3 = false;
                    bl4 = false;
                    bl2 = false;
                    n5 = 0;
                }
                bl4 = true;
                continue;
            }
            if (c2 == '+') {
                if (!bl2) continue;
                n2 = aen.a(n2, n5, bl4, bl3, bl5);
                bl5 = false;
                bl3 = false;
                bl4 = false;
                bl2 = false;
                n5 = 0;
                continue;
            }
            if (c2 != '&') continue;
            if (bl2) {
                n2 = aen.a(n2, n5, bl4, bl3, bl5);
                bl5 = false;
                bl3 = false;
                bl4 = false;
                bl2 = false;
                n5 = 0;
            }
            bl5 = true;
        }
        if (bl2) {
            n2 = aen.a(n2, n5, bl4, bl3, bl5);
        }
        return n2 & Short.MAX_VALUE;
    }

    public static int a(int n2, int n3, int n4, int n5, int n6, int n7) {
        return (aen.a(n2, n3) ? 16 : 0) | (aen.a(n2, n4) ? 8 : 0) | (aen.a(n2, n5) ? 4 : 0) | (aen.a(n2, n6) ? 2 : 0) | (aen.a(n2, n7) ? 1 : 0);
    }

    static {
        n = new HashMap();
        o = new HashMap();
        a = null;
        c = "+0-1-2-3&4-4+13";
        n.put(rv.l.c(), "0 & !1 & !2 & !3 & 0+6");
        b = "-0+1-2-3&4-4+13";
        n.put(rv.c.c(), "!0 & 1 & !2 & !3 & 1+6");
        h = "+0+1-2-3&4-4+13";
        n.put(rv.n.c(), "0 & 1 & !2 & !3 & 0+6");
        f = "+0-1+2-3&4-4+13";
        n.put(rv.h.c(), "0 & !1 & 2 & !3");
        d = "-0-1+2-3&4-4+13";
        n.put(rv.u.c(), "!0 & !1 & 2 & !3 & 2+6");
        e = "-0+3-4+13";
        n.put(rv.t.c(), "!0 & !1 & !2 & 3 & 3+6");
        n.put(rv.i.c(), "!0 & !1 & 2 & 3");
        n.put(rv.d.c(), "!0 & 1 & !2 & 3 & 3+6");
        g = "+0-1-2+3&4-4+13";
        n.put(rv.g.c(), "0 & !1 & !2 & 3 & 3+6");
        l = "-0+1+2-3+13&4-4";
        n.put(rv.r.c(), "!0 & 1 & 2 & !3 & 2+6");
        n.put(rv.p.c(), "!0 & 1 & 2 & 3 & 2+6");
        m = "+0-1+2+3+13&4-4";
        n.put(rv.o.c(), "0 & !1 & 2 & 3 & 2+6");
        j = "+5-6-7";
        o.put(rv.c.c(), "5");
        o.put(rv.e.c(), "5");
        o.put(rv.g.c(), "5");
        o.put(rv.l.c(), "5");
        o.put(rv.i.c(), "5");
        o.put(rv.h.c(), "5");
        o.put(rv.m.c(), "5");
        o.put(rv.u.c(), "5");
        i = "-5+6-7";
        k = "+14&13-13";
        p = new HashMap();
        q = new String[]{"potion.prefix.mundane", "potion.prefix.uninteresting", "potion.prefix.bland", "potion.prefix.clear", "potion.prefix.milky", "potion.prefix.diffuse", "potion.prefix.artless", "potion.prefix.thin", "potion.prefix.awkward", "potion.prefix.flat", "potion.prefix.bulky", "potion.prefix.bungling", "potion.prefix.buttered", "potion.prefix.smooth", "potion.prefix.suave", "potion.prefix.debonair", "potion.prefix.thick", "potion.prefix.elegant", "potion.prefix.fancy", "potion.prefix.charming", "potion.prefix.dashing", "potion.prefix.refined", "potion.prefix.cordial", "potion.prefix.sparkling", "potion.prefix.potent", "potion.prefix.foul", "potion.prefix.odorless", "potion.prefix.rank", "potion.prefix.harsh", "potion.prefix.acrid", "potion.prefix.gross", "potion.prefix.stinky"};
    }
}

