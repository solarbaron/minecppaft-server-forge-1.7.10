/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.primitives.Doubles;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import net.minecraft.server.MinecraftServer;

public abstract class y
implements aa {
    private static x a;

    public int a() {
        return 4;
    }

    @Override
    public List b() {
        return null;
    }

    @Override
    public boolean a(ac ac2) {
        return ac2.a(this.a(), this.c());
    }

    @Override
    public List a(ac ac2, String[] stringArray) {
        return null;
    }

    public static int a(ac ac2, String string) {
        try {
            return Integer.parseInt(string);
        }
        catch (NumberFormatException numberFormatException) {
            throw new ce("commands.generic.num.invalid", string);
        }
    }

    public static int a(ac ac2, String string, int n2) {
        return y.a(ac2, string, n2, Integer.MAX_VALUE);
    }

    public static int a(ac ac2, String string, int n2, int n3) {
        int n4 = y.a(ac2, string);
        if (n4 < n2) {
            throw new ce("commands.generic.num.tooSmall", n4, n2);
        }
        if (n4 > n3) {
            throw new ce("commands.generic.num.tooBig", n4, n3);
        }
        return n4;
    }

    public static double b(ac ac2, String string) {
        try {
            double d2 = Double.parseDouble(string);
            if (!Doubles.isFinite(d2)) {
                throw new ce("commands.generic.num.invalid", string);
            }
            return d2;
        }
        catch (NumberFormatException numberFormatException) {
            throw new ce("commands.generic.num.invalid", string);
        }
    }

    public static double a(ac ac2, String string, double d2) {
        return y.a(ac2, string, d2, Double.MAX_VALUE);
    }

    public static double a(ac ac2, String string, double d2, double d3) {
        double d4 = y.b(ac2, string);
        if (d4 < d2) {
            throw new ce("commands.generic.double.tooSmall", d4, d2);
        }
        if (d4 > d3) {
            throw new ce("commands.generic.double.tooBig", d4, d3);
        }
        return d4;
    }

    public static boolean c(ac ac2, String string) {
        if (string.equals("true") || string.equals("1")) {
            return true;
        }
        if (string.equals("false") || string.equals("0")) {
            return false;
        }
        throw new cd("commands.generic.boolean.invalid", string);
    }

    public static mw b(ac ac2) {
        if (ac2 instanceof mw) {
            return (mw)ac2;
        }
        throw new cg("You must specify which player you wish to perform this action on.", new Object[0]);
    }

    public static mw d(ac ac2, String string) {
        mw mw2 = ad.a(ac2, string);
        if (mw2 != null) {
            return mw2;
        }
        mw2 = MinecraftServer.I().ah().a(string);
        if (mw2 == null) {
            throw new cg();
        }
        return mw2;
    }

    public static String e(ac ac2, String string) {
        mw mw2 = ad.a(ac2, string);
        if (mw2 != null) {
            return mw2.b_();
        }
        if (ad.b(string)) {
            throw new cg();
        }
        return string;
    }

    public static fj a(ac ac2, String[] stringArray, int n2) {
        return y.a(ac2, stringArray, n2, false);
    }

    public static fj a(ac ac2, String[] stringArray, int n2, boolean bl2) {
        fq fq2 = new fq("");
        for (int i2 = n2; i2 < stringArray.length; ++i2) {
            if (i2 > n2) {
                fq2.a(" ");
            }
            fj fj2 = new fq(stringArray[i2]);
            if (bl2) {
                fj fj3 = ad.b(ac2, stringArray[i2]);
                if (fj3 != null) {
                    fj2 = fj3;
                } else if (ad.b(stringArray[i2])) {
                    throw new cg();
                }
            }
            fq2.a(fj2);
        }
        return fq2;
    }

    public static String b(ac ac2, String[] stringArray, int n2) {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i2 = n2; i2 < stringArray.length; ++i2) {
            if (i2 > n2) {
                stringBuilder.append(" ");
            }
            String string = stringArray[i2];
            stringBuilder.append(string);
        }
        return stringBuilder.toString();
    }

    public static double a(ac ac2, double d2, String string) {
        return y.a(ac2, d2, string, -30000000, 30000000);
    }

    public static double a(ac ac2, double d2, String string, int n2, int n3) {
        double d3;
        boolean bl2 = string.startsWith("~");
        if (bl2 && Double.isNaN(d2)) {
            throw new ce("commands.generic.num.invalid", d2);
        }
        double d4 = d3 = bl2 ? d2 : 0.0;
        if (!bl2 || string.length() > 1) {
            boolean bl3 = string.contains(".");
            if (bl2) {
                string = string.substring(1);
            }
            d3 += y.b(ac2, string);
            if (!bl3 && !bl2) {
                d3 += 0.5;
            }
        }
        if (n2 != 0 || n3 != 0) {
            if (d3 < (double)n2) {
                throw new ce("commands.generic.double.tooSmall", d3, n2);
            }
            if (d3 > (double)n3) {
                throw new ce("commands.generic.double.tooBig", d3, n3);
            }
        }
        return d3;
    }

    public static adb f(ac ac2, String string) {
        adb adb2 = (adb)adb.e.a(string);
        if (adb2 == null) {
            try {
                adb adb3 = adb.d(Integer.parseInt(string));
                if (adb3 != null) {
                    fr fr2 = new fr("commands.generic.deprecatedId", adb.e.c(adb3));
                    fr2.b().a(a.h);
                    ac2.a(fr2);
                }
                adb2 = adb3;
            }
            catch (NumberFormatException numberFormatException) {
                // empty catch block
            }
        }
        if (adb2 == null) {
            throw new ce("commands.give.notFound", string);
        }
        return adb2;
    }

    public static aji g(ac ac2, String string) {
        if (aji.c.b(string)) {
            return (aji)aji.c.a(string);
        }
        try {
            int n2 = Integer.parseInt(string);
            if (aji.c.b(n2)) {
                aji aji2 = aji.e(n2);
                fr fr2 = new fr("commands.generic.deprecatedId", aji.c.c(aji2));
                fr2.b().a(a.h);
                ac2.a(fr2);
                return aji2;
            }
        }
        catch (NumberFormatException numberFormatException) {
            // empty catch block
        }
        throw new ce("commands.give.notFound", string);
    }

    public static String a(Object[] objectArray) {
        StringBuilder stringBuilder = new StringBuilder();
        for (int i2 = 0; i2 < objectArray.length; ++i2) {
            String string = objectArray[i2].toString();
            if (i2 > 0) {
                if (i2 == objectArray.length - 1) {
                    stringBuilder.append(" and ");
                } else {
                    stringBuilder.append(", ");
                }
            }
            stringBuilder.append(string);
        }
        return stringBuilder.toString();
    }

    public static fj a(fj[] fjArray) {
        fq fq2 = new fq("");
        for (int i2 = 0; i2 < fjArray.length; ++i2) {
            if (i2 > 0) {
                if (i2 == fjArray.length - 1) {
                    fq2.a(" and ");
                } else if (i2 > 0) {
                    fq2.a(", ");
                }
            }
            fq2.a(fjArray[i2]);
        }
        return fq2;
    }

    public static String a(Collection collection) {
        return y.a(collection.toArray(new String[collection.size()]));
    }

    public static boolean a(String string, String string2) {
        return string2.regionMatches(true, 0, string, 0, string.length());
    }

    public static List a(String[] stringArray, String ... stringArray2) {
        String string = stringArray[stringArray.length - 1];
        ArrayList<String> arrayList = new ArrayList<String>();
        for (String string2 : stringArray2) {
            if (!y.a(string, string2)) continue;
            arrayList.add(string2);
        }
        return arrayList;
    }

    public static List a(String[] stringArray, Iterable iterable) {
        String string = stringArray[stringArray.length - 1];
        ArrayList<String> arrayList = new ArrayList<String>();
        for (String string2 : iterable) {
            if (!y.a(string, string2)) continue;
            arrayList.add(string2);
        }
        return arrayList;
    }

    @Override
    public boolean a(String[] stringArray, int n2) {
        return false;
    }

    public static void a(ac ac2, aa aa2, String string, Object ... objectArray) {
        y.a(ac2, aa2, 0, string, objectArray);
    }

    public static void a(ac ac2, aa aa2, int n2, String string, Object ... objectArray) {
        if (a != null) {
            a.a(ac2, aa2, n2, string, objectArray);
        }
    }

    public static void a(x x2) {
        a = x2;
    }

    public int a(aa aa2) {
        return this.c().compareTo(aa2.c());
    }

    public /* synthetic */ int compareTo(Object object) {
        return this.a((aa)object);
    }
}

