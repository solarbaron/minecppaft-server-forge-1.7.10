/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;

public enum fi {
    a("open_url", true),
    b("open_file", false),
    c("run_command", true),
    d("twitch_user_info", false),
    e("suggest_command", true);

    private static final Map f;
    private final boolean g;
    private final String h;

    /*
     * WARNING - void declaration
     */
    private fi() {
        void var4_1;
        void var3_2;
        this.h = var3_2;
        this.g = var4_1;
    }

    public boolean a() {
        return this.g;
    }

    public String b() {
        return this.h;
    }

    public static fi a(String string) {
        return (fi)((Object)f.get(string));
    }

    static {
        f = Maps.newHashMap();
        for (fi fi2 : fi.values()) {
            f.put(fi2.b(), fi2);
        }
    }
}

