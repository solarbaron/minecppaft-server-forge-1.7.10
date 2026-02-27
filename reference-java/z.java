/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class z
implements ab {
    private static final Logger a = LogManager.getLogger();
    private final Map b = new HashMap();
    private final Set c = new HashSet();

    @Override
    public int a(ac ac2, String string) {
        if ((string = string.trim()).startsWith("/")) {
            string = string.substring(1);
        }
        String[] stringArray = string.split(" ");
        String string2 = stringArray[0];
        stringArray = z.a(stringArray);
        aa aa2 = (aa)this.b.get(string2);
        int n2 = this.a(aa2, stringArray);
        int n3 = 0;
        try {
            if (aa2 == null) {
                throw new ch();
            }
            if (aa2.a(ac2)) {
                if (n2 > -1) {
                    mw[] mwArray = ad.c(ac2, stringArray[n2]);
                    String string3 = stringArray[n2];
                    for (mw mw2 : mwArray) {
                        stringArray[n2] = mw2.b_();
                        try {
                            aa2.b(ac2, stringArray);
                            ++n3;
                        }
                        catch (cd cd2) {
                            fr fr2 = new fr(cd2.getMessage(), cd2.a());
                            fr2.b().a(a.m);
                            ac2.a(fr2);
                        }
                    }
                    stringArray[n2] = string3;
                } else {
                    try {
                        aa2.b(ac2, stringArray);
                        ++n3;
                    }
                    catch (cd cd3) {
                        fr fr3 = new fr(cd3.getMessage(), cd3.a());
                        fr3.b().a(a.m);
                        ac2.a(fr3);
                    }
                }
            } else {
                fr fr4 = new fr("commands.generic.permission", new Object[0]);
                fr4.b().a(a.m);
                ac2.a(fr4);
            }
        }
        catch (ci ci2) {
            fr fr5 = new fr("commands.generic.usage", new fr(ci2.getMessage(), ci2.a()));
            fr5.b().a(a.m);
            ac2.a(fr5);
        }
        catch (cd cd4) {
            fr fr6 = new fr(cd4.getMessage(), cd4.a());
            fr6.b().a(a.m);
            ac2.a(fr6);
        }
        catch (Throwable throwable) {
            fr fr7 = new fr("commands.generic.exception", new Object[0]);
            fr7.b().a(a.m);
            ac2.a(fr7);
            a.error("Couldn't process command: '" + string + "'", throwable);
        }
        return n3;
    }

    public aa a(aa aa2) {
        List list = aa2.b();
        this.b.put(aa2.c(), aa2);
        this.c.add(aa2);
        if (list != null) {
            for (String string : list) {
                aa aa3 = (aa)this.b.get(string);
                if (aa3 != null && aa3.c().equals(string)) continue;
                this.b.put(string, aa2);
            }
        }
        return aa2;
    }

    private static String[] a(String[] stringArray) {
        String[] stringArray2 = new String[stringArray.length - 1];
        for (int i2 = 1; i2 < stringArray.length; ++i2) {
            stringArray2[i2 - 1] = stringArray[i2];
        }
        return stringArray2;
    }

    @Override
    public List b(ac ac2, String string) {
        aa aa2;
        String[] stringArray = string.split(" ", -1);
        String string2 = stringArray[0];
        if (stringArray.length == 1) {
            ArrayList arrayList = new ArrayList();
            for (Map.Entry entry : this.b.entrySet()) {
                if (!y.a(string2, (String)entry.getKey()) || !((aa)entry.getValue()).a(ac2)) continue;
                arrayList.add(entry.getKey());
            }
            return arrayList;
        }
        if (stringArray.length > 1 && (aa2 = (aa)this.b.get(string2)) != null) {
            return aa2.a(ac2, z.a(stringArray));
        }
        return null;
    }

    @Override
    public List a(ac ac2) {
        ArrayList<aa> arrayList = new ArrayList<aa>();
        for (aa aa2 : this.c) {
            if (!aa2.a(ac2)) continue;
            arrayList.add(aa2);
        }
        return arrayList;
    }

    @Override
    public Map a() {
        return this.b;
    }

    private int a(aa aa2, String[] stringArray) {
        if (aa2 == null) {
            return -1;
        }
        for (int i2 = 0; i2 < stringArray.length; ++i2) {
            if (!aa2.a(stringArray, i2) || !ad.a(stringArray[i2])) continue;
            return i2;
        }
        return -1;
    }
}

