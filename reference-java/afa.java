/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashMap;
import java.util.Map;

public class afa {
    private static final afa a = new afa();
    private Map b = new HashMap();
    private Map c = new HashMap();

    public static afa a() {
        return a;
    }

    private afa() {
        this.a(ajn.p, new add(ade.j), 0.7f);
        this.a(ajn.o, new add(ade.k), 1.0f);
        this.a(ajn.ag, new add(ade.i), 1.0f);
        this.a(ajn.m, new add(ajn.w), 0.1f);
        this.a(ade.al, new add(ade.am), 0.35f);
        this.a(ade.bd, new add(ade.be), 0.35f);
        this.a(ade.bf, new add(ade.bg), 0.35f);
        this.a(ajn.e, new add(ajn.b), 0.1f);
        this.a(ade.aD, new add(ade.aC), 0.3f);
        this.a(ajn.aG, new add(ajn.ch), 0.35f);
        this.a(ajn.aF, new add(ade.aR, 1, 2), 0.2f);
        this.a(ajn.r, new add(ade.h, 1, 1), 0.15f);
        this.a(ajn.s, new add(ade.h, 1, 1), 0.15f);
        this.a(ajn.bA, new add(ade.bC), 1.0f);
        this.a(ade.bG, new add(ade.bH), 0.35f);
        this.a(ajn.aL, new add(ade.bT), 0.1f);
        for (acu acu2 : acu.values()) {
            if (!acu2.i()) continue;
            this.a(new add(ade.aP, 1, acu2.a()), new add(ade.aQ, 1, acu2.a()), 0.35f);
        }
        this.a(ajn.q, new add(ade.h), 0.1f);
        this.a(ajn.ax, new add(ade.ax), 0.7f);
        this.a(ajn.x, new add(ade.aR, 1, 4), 0.2f);
        this.a(ajn.bY, new add(ade.bU), 0.2f);
    }

    public void a(aji aji2, add add2, float f2) {
        this.a(adb.a(aji2), add2, f2);
    }

    public void a(adb adb2, add add2, float f2) {
        this.a(new add(adb2, 1, Short.MAX_VALUE), add2, f2);
    }

    public void a(add add2, add add3, float f2) {
        this.b.put(add2, add3);
        this.c.put(add3, Float.valueOf(f2));
    }

    public add a(add add2) {
        for (Map.Entry entry : this.b.entrySet()) {
            if (!this.a(add2, (add)entry.getKey())) continue;
            return (add)entry.getValue();
        }
        return null;
    }

    private boolean a(add add2, add add3) {
        return add3.b() == add2.b() && (add3.k() == Short.MAX_VALUE || add3.k() == add2.k());
    }

    public Map b() {
        return this.b;
    }

    public float b(add add2) {
        for (Map.Entry entry : this.c.entrySet()) {
            if (!this.a(add2, (add)entry.getKey())) continue;
            return ((Float)entry.getValue()).floatValue();
        }
        return 0.0f;
    }
}

