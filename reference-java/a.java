/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public enum a {
    a('0'),
    b('1'),
    c('2'),
    d('3'),
    e('4'),
    f('5'),
    g('6'),
    h('7'),
    i('8'),
    j('9'),
    k('a'),
    l('b'),
    m('c'),
    n('d'),
    o('e'),
    p('f'),
    q('k', true),
    r('l', true),
    s('m', true),
    t('n', true),
    u('o', true),
    v('r');

    private static final Map w;
    private static final Map x;
    private static final Pattern y;
    private final char z;
    private final boolean A;
    private final String B;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private a() {
        this((String)var1_-1, (int)var2_-1, (char)var3_2, false);
        void var3_2;
        void var2_-1;
        void var1_-1;
    }

    /*
     * WARNING - void declaration
     */
    private a() {
        void var4_1;
        void var3_2;
        this.z = var3_2;
        this.A = var4_1;
        this.B = "\u00a7" + (char)var3_2;
    }

    public char a() {
        return this.z;
    }

    public boolean b() {
        return this.A;
    }

    public boolean c() {
        return !this.A && this != v;
    }

    public String d() {
        return this.name().toLowerCase();
    }

    public String toString() {
        return this.B;
    }

    public static a b(String string) {
        if (string == null) {
            return null;
        }
        return (a)((Object)x.get(string.toLowerCase()));
    }

    public static Collection a(boolean bl2, boolean bl3) {
        ArrayList<String> arrayList = new ArrayList<String>();
        for (a a2 : a.values()) {
            if (a2.c() && !bl2 || a2.b() && !bl3) continue;
            arrayList.add(a2.d());
        }
        return arrayList;
    }

    static {
        w = new HashMap();
        x = new HashMap();
        y = Pattern.compile("(?i)" + String.valueOf('\u00a7') + "[0-9A-FK-OR]");
        for (a a2 : a.values()) {
            w.put(Character.valueOf(a2.a()), a2);
            x.put(a2.d(), a2);
        }
    }
}

