/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.collect.Maps;
import java.util.Map;

public enum acu {
    a(0, "cod", 2, 0.1f, 5, 0.6f),
    b(1, "salmon", 2, 0.1f, 6, 0.8f),
    c(2, "clownfish", 1, 0.1f),
    d(3, "pufferfish", 1, 0.1f);

    private static final Map e;
    private final int f;
    private final String g;
    private final int j;
    private final float k;
    private final int l;
    private final float m;
    private boolean n = false;

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private acu(int n3, float f2, int n4, float f3) {
        void var8_6;
        void var7_5;
        this.f = n3;
        this.g = (String)f2;
        this.j = n4;
        this.k = f3;
        this.l = var7_5;
        this.m = var8_6;
        this.n = true;
    }

    /*
     * WARNING - Possible parameter corruption
     * WARNING - void declaration
     */
    private acu(int n3, float f2) {
        void var6_4;
        void var5_3;
        this.f = n3;
        this.g = (String)f2;
        this.j = var5_3;
        this.k = var6_4;
        this.l = 0;
        this.m = 0.0f;
        this.n = false;
    }

    public int a() {
        return this.f;
    }

    public String b() {
        return this.g;
    }

    public int c() {
        return this.j;
    }

    public float d() {
        return this.k;
    }

    public int e() {
        return this.l;
    }

    public float f() {
        return this.m;
    }

    public boolean i() {
        return this.n;
    }

    public static acu a(int n2) {
        acu acu2 = (acu)((Object)e.get(n2));
        if (acu2 == null) {
            return a;
        }
        return acu2;
    }

    public static acu a(add add2) {
        if (add2.b() instanceof act) {
            return acu.a(add2.k());
        }
        return a;
    }

    static {
        e = Maps.newHashMap();
        for (acu acu2 : acu.values()) {
            e.put(acu2.a(), acu2);
        }
    }
}

