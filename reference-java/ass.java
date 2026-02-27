/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ass {
    private final List a = new ArrayList();
    private final Map b = new HashMap();
    private int c;

    public int a() {
        return this.c;
    }

    public void a(int n2) {
        this.c = n2;
    }

    public Map b() {
        return this.b;
    }

    public List c() {
        return this.a;
    }

    public void d() {
        int n2 = 0;
        for (ast ast2 : this.a) {
            ast2.c(n2);
            n2 += ast2.a();
        }
    }

    public String toString() {
        int n2;
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(2);
        stringBuilder.append(";");
        for (n2 = 0; n2 < this.a.size(); ++n2) {
            if (n2 > 0) {
                stringBuilder.append(",");
            }
            stringBuilder.append(((ast)this.a.get(n2)).toString());
        }
        stringBuilder.append(";");
        stringBuilder.append(this.c);
        if (!this.b.isEmpty()) {
            stringBuilder.append(";");
            n2 = 0;
            for (Map.Entry entry : this.b.entrySet()) {
                if (n2++ > 0) {
                    stringBuilder.append(",");
                }
                stringBuilder.append(((String)entry.getKey()).toLowerCase());
                Map map = (Map)entry.getValue();
                if (map.isEmpty()) continue;
                stringBuilder.append("(");
                int n3 = 0;
                for (Map.Entry entry2 : map.entrySet()) {
                    if (n3++ > 0) {
                        stringBuilder.append(" ");
                    }
                    stringBuilder.append((String)entry2.getKey());
                    stringBuilder.append("=");
                    stringBuilder.append((String)entry2.getValue());
                }
                stringBuilder.append(")");
            }
        } else {
            stringBuilder.append(";");
        }
        return stringBuilder.toString();
    }

    private static ast a(String string, int n2) {
        int n3;
        Object object;
        String[] stringArray = string.split("x", 2);
        int n4 = 1;
        int n5 = 0;
        if (stringArray.length == 2) {
            try {
                n4 = Integer.parseInt(stringArray[0]);
                if (n2 + n4 >= 256) {
                    n4 = 256 - n2;
                }
                if (n4 < 0) {
                    n4 = 0;
                }
            }
            catch (Throwable throwable) {
                return null;
            }
        }
        try {
            object = stringArray[stringArray.length - 1];
            stringArray = ((String)object).split(":", 2);
            n3 = Integer.parseInt(stringArray[0]);
            if (stringArray.length > 1) {
                n5 = Integer.parseInt(stringArray[1]);
            }
            if (aji.e(n3) == ajn.a) {
                n3 = 0;
                n5 = 0;
            }
            if (n5 < 0 || n5 > 15) {
                n5 = 0;
            }
        }
        catch (Throwable throwable) {
            return null;
        }
        object = new ast(n4, aji.e(n3), n5);
        ((ast)object).c(n2);
        return object;
    }

    private static List b(String string) {
        if (string == null || string.length() < 1) {
            return null;
        }
        ArrayList<ast> arrayList = new ArrayList<ast>();
        String[] stringArray = string.split(",");
        int n2 = 0;
        for (String string2 : stringArray) {
            ast ast2 = ass.a(string2, n2);
            if (ast2 == null) {
                return null;
            }
            arrayList.add(ast2);
            n2 += ast2.a();
        }
        return arrayList;
    }

    public static ass a(String string) {
        List list;
        int n2;
        if (string == null) {
            return ass.e();
        }
        String[] stringArray = string.split(";", -1);
        int n3 = n2 = stringArray.length == 1 ? 0 : qh.a(stringArray[0], 0);
        if (n2 < 0 || n2 > 2) {
            return ass.e();
        }
        ass ass2 = new ass();
        int n4 = stringArray.length == 1 ? 0 : 1;
        if ((list = ass.b(stringArray[n4++])) == null || list.isEmpty()) {
            return ass.e();
        }
        ass2.c().addAll(list);
        ass2.d();
        int n5 = ahu.p.ay;
        if (n2 > 0 && stringArray.length > n4) {
            n5 = qh.a(stringArray[n4++], n5);
        }
        ass2.a(n5);
        if (n2 > 0 && stringArray.length > n4) {
            String[] stringArray2;
            for (String string2 : stringArray2 = stringArray[n4++].toLowerCase().split(",")) {
                String[] stringArray3 = string2.split("\\(", 2);
                HashMap<String, String> hashMap = new HashMap<String, String>();
                if (stringArray3[0].length() <= 0) continue;
                ass2.b().put(stringArray3[0], hashMap);
                if (stringArray3.length <= 1 || !stringArray3[1].endsWith(")") || stringArray3[1].length() <= 1) continue;
                String[] stringArray4 = stringArray3[1].substring(0, stringArray3[1].length() - 1).split(" ");
                for (int i2 = 0; i2 < stringArray4.length; ++i2) {
                    String[] stringArray5 = stringArray4[i2].split("=", 2);
                    if (stringArray5.length != 2) continue;
                    hashMap.put(stringArray5[0], stringArray5[1]);
                }
            }
        } else {
            ass2.b().put("village", new HashMap());
        }
        return ass2;
    }

    public static ass e() {
        ass ass2 = new ass();
        ass2.a(ahu.p.ay);
        ass2.c().add(new ast(1, ajn.h));
        ass2.c().add(new ast(2, ajn.d));
        ass2.c().add(new ast(1, ajn.c));
        ass2.d();
        ass2.b().put("village", new HashMap());
        return ass2;
    }
}

