/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import net.minecraft.server.MinecraftServer;

public class ad {
    private static final Pattern a = Pattern.compile("^@([parf])(?:\\[([\\w=,!-]*)\\])?$");
    private static final Pattern b = Pattern.compile("\\G([-!]?[\\w-]*)(?:$|,)");
    private static final Pattern c = Pattern.compile("\\G(\\w+)=([-!]?[\\w-]*)(?:$|,)");

    public static mw a(ac ac2, String string) {
        mw[] mwArray = ad.c(ac2, string);
        if (mwArray == null || mwArray.length != 1) {
            return null;
        }
        return mwArray[0];
    }

    public static fj b(ac ac2, String string) {
        mw[] mwArray = ad.c(ac2, string);
        if (mwArray == null || mwArray.length == 0) {
            return null;
        }
        fj[] fjArray = new fj[mwArray.length];
        for (int i2 = 0; i2 < fjArray.length; ++i2) {
            fjArray[i2] = mwArray[i2].c_();
        }
        return y.a(fjArray);
    }

    public static mw[] c(ac ac2, String string) {
        Matcher matcher = a.matcher(string);
        if (matcher.matches()) {
            ahb ahb2;
            Map map = ad.h(matcher.group(2));
            String string2 = matcher.group(1);
            int n2 = ad.c(string2);
            int n3 = ad.d(string2);
            int n4 = ad.f(string2);
            int n5 = ad.e(string2);
            int n6 = ad.g(string2);
            int n7 = ahk.a.a();
            r r2 = ac2.f_();
            Map map2 = ad.a(map);
            String string3 = null;
            String string4 = null;
            boolean bl2 = false;
            if (map.containsKey("rm")) {
                n2 = qh.a((String)map.get("rm"), n2);
                bl2 = true;
            }
            if (map.containsKey("r")) {
                n3 = qh.a((String)map.get("r"), n3);
                bl2 = true;
            }
            if (map.containsKey("lm")) {
                n4 = qh.a((String)map.get("lm"), n4);
            }
            if (map.containsKey("l")) {
                n5 = qh.a((String)map.get("l"), n5);
            }
            if (map.containsKey("x")) {
                r2.a = qh.a((String)map.get("x"), r2.a);
                bl2 = true;
            }
            if (map.containsKey("y")) {
                r2.b = qh.a((String)map.get("y"), r2.b);
                bl2 = true;
            }
            if (map.containsKey("z")) {
                r2.c = qh.a((String)map.get("z"), r2.c);
                bl2 = true;
            }
            if (map.containsKey("m")) {
                n7 = qh.a((String)map.get("m"), n7);
            }
            if (map.containsKey("c")) {
                n6 = qh.a((String)map.get("c"), n6);
            }
            if (map.containsKey("team")) {
                string4 = (String)map.get("team");
            }
            if (map.containsKey("name")) {
                string3 = (String)map.get("name");
            }
            ahb ahb3 = ahb2 = bl2 ? ac2.d() : null;
            if (string2.equals("p") || string2.equals("a")) {
                List list = MinecraftServer.I().ah().a(r2, n2, n3, n6, n7, n4, n5, map2, string3, string4, ahb2);
                return list.isEmpty() ? new mw[]{} : list.toArray(new mw[list.size()]);
            }
            if (string2.equals("r")) {
                List list = MinecraftServer.I().ah().a(r2, n2, n3, 0, n7, n4, n5, map2, string3, string4, ahb2);
                Collections.shuffle(list);
                list = list.subList(0, Math.min(n6, list.size()));
                return list.isEmpty() ? new mw[]{} : list.toArray(new mw[list.size()]);
            }
            return null;
        }
        return null;
    }

    public static Map a(Map map) {
        HashMap<String, Integer> hashMap = new HashMap<String, Integer>();
        for (String string : map.keySet()) {
            if (!string.startsWith("score_") || string.length() <= "score_".length()) continue;
            String string2 = string.substring("score_".length());
            hashMap.put(string2, qh.a((String)map.get(string), 1));
        }
        return hashMap;
    }

    public static boolean a(String string) {
        Matcher matcher = a.matcher(string);
        if (matcher.matches()) {
            Map map = ad.h(matcher.group(2));
            String string2 = matcher.group(1);
            int n2 = ad.g(string2);
            if (map.containsKey("c")) {
                n2 = qh.a((String)map.get("c"), n2);
            }
            return n2 != 1;
        }
        return false;
    }

    public static boolean a(String string, String string2) {
        Matcher matcher = a.matcher(string);
        if (matcher.matches()) {
            String string3 = matcher.group(1);
            return string2 == null || string2.equals(string3);
        }
        return false;
    }

    public static boolean b(String string) {
        return ad.a(string, null);
    }

    private static final int c(String string) {
        return 0;
    }

    private static final int d(String string) {
        return 0;
    }

    private static final int e(String string) {
        return Integer.MAX_VALUE;
    }

    private static final int f(String string) {
        return 0;
    }

    private static final int g(String string) {
        if (string.equals("a")) {
            return 0;
        }
        return 1;
    }

    private static Map h(String string) {
        HashMap<String, String> hashMap = new HashMap<String, String>();
        if (string == null) {
            return hashMap;
        }
        Matcher matcher = b.matcher(string);
        int n2 = 0;
        int n3 = -1;
        while (matcher.find()) {
            String string2 = null;
            switch (n2++) {
                case 0: {
                    string2 = "x";
                    break;
                }
                case 1: {
                    string2 = "y";
                    break;
                }
                case 2: {
                    string2 = "z";
                    break;
                }
                case 3: {
                    string2 = "r";
                }
            }
            if (string2 != null && matcher.group(1).length() > 0) {
                hashMap.put(string2, matcher.group(1));
            }
            n3 = matcher.end();
        }
        if (n3 < string.length()) {
            matcher = c.matcher(n3 == -1 ? string : string.substring(n3));
            while (matcher.find()) {
                hashMap.put(matcher.group(1), matcher.group(2));
            }
        }
        return hashMap;
    }
}

