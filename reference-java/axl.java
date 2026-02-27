/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;

public class axl {
    private static int a = 256;
    private static List b = new ArrayList();
    private static List c = new ArrayList();
    private static List d = new ArrayList();
    private static List e = new ArrayList();

    public static synchronized int[] a(int n2) {
        if (n2 <= 256) {
            if (b.isEmpty()) {
                int[] nArray = new int[256];
                c.add(nArray);
                return nArray;
            }
            int[] nArray = (int[])b.remove(b.size() - 1);
            c.add(nArray);
            return nArray;
        }
        if (n2 > a) {
            a = n2;
            d.clear();
            e.clear();
            int[] nArray = new int[a];
            e.add(nArray);
            return nArray;
        }
        if (d.isEmpty()) {
            int[] nArray = new int[a];
            e.add(nArray);
            return nArray;
        }
        int[] nArray = (int[])d.remove(d.size() - 1);
        e.add(nArray);
        return nArray;
    }

    public static synchronized void a() {
        if (!d.isEmpty()) {
            d.remove(d.size() - 1);
        }
        if (!b.isEmpty()) {
            b.remove(b.size() - 1);
        }
        d.addAll(e);
        b.addAll(c);
        e.clear();
        c.clear();
    }

    public static synchronized String b() {
        return "cache: " + d.size() + ", tcache: " + b.size() + ", allocated: " + e.size() + ", tallocated: " + c.size();
    }
}

