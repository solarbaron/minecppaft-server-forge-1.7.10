/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;

public enum fm {
    a("show_text", true),
    b("show_achievement", true),
    c("show_item", true);

    private static final Map d;
    private final boolean e;
    private final String f;

    /*
     * WARNING - void declaration
     */
    private fm() {
        void var4_1;
        void var3_2;
        this.f = var3_2;
        this.e = var4_1;
    }

    public boolean a() {
        return this.e;
    }

    public String b() {
        return this.f;
    }

    public static fm a(String string) {
        return (fm)((Object)d.get(string));
    }

    static {
        d = Maps.newHashMap();
        for (fm fm2 : fm.values()) {
            d.put(fm2.b(), fm2);
        }
    }
}

