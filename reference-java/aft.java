/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public abstract class aft {
    public static final aft[] b = new aft[256];
    public static final aft[] c;
    public static final aft d;
    public static final aft e;
    public static final aft f;
    public static final aft g;
    public static final aft h;
    public static final aft i;
    public static final aft j;
    public static final aft k;
    public static final aft l;
    public static final aft m;
    public static final aft n;
    public static final aft o;
    public static final aft p;
    public static final aft q;
    public static final aft r;
    public static final aft s;
    public static final aft t;
    public static final aft u;
    public static final aft v;
    public static final aft w;
    public static final aft x;
    public static final aft y;
    public static final aft z;
    public static final aft A;
    public final int B;
    private final int a;
    public afu C;
    protected String D;

    protected aft(int n2, int n3, afu afu2) {
        this.B = n2;
        this.a = n3;
        this.C = afu2;
        if (b[n2] != null) {
            throw new IllegalArgumentException("Duplicate enchantment id!");
        }
        aft.b[n2] = this;
    }

    public int c() {
        return this.a;
    }

    public int d() {
        return 1;
    }

    public int b() {
        return 1;
    }

    public int a(int n2) {
        return 1 + n2 * 10;
    }

    public int b(int n2) {
        return this.a(n2) + 5;
    }

    public int a(int n2, ro ro2) {
        return 0;
    }

    public float a(int n2, sz sz2) {
        return 0.0f;
    }

    public boolean a(aft aft2) {
        return this != aft2;
    }

    public aft b(String string) {
        this.D = string;
        return this;
    }

    public String a() {
        return "enchantment." + this.D;
    }

    public String c(int n2) {
        String string = dd.a(this.a());
        return string + " " + dd.a("enchantment.level." + n2);
    }

    public boolean a(add add2) {
        return this.C.a(add2.b());
    }

    public void a(sv sv2, sa sa2, int n2) {
    }

    public void b(sv sv2, sa sa2, int n2) {
    }

    static {
        d = new agi(0, 10, 0);
        e = new agi(1, 5, 1);
        f = new agi(2, 5, 2);
        g = new agi(3, 2, 3);
        h = new agi(4, 5, 4);
        i = new agh(5, 2);
        j = new agl(6, 2);
        k = new agj(7, 1);
        l = new afq(16, 10, 0);
        m = new afq(17, 5, 1);
        n = new afq(18, 5, 2);
        o = new agf(19, 5);
        p = new agd(20, 2);
        q = new agg(21, 2, afu.g);
        r = new afs(32, 10);
        s = new agk(33, 1);
        t = new afr(34, 5);
        u = new agg(35, 2, afu.h);
        v = new afm(48, 10);
        w = new afp(49, 2);
        x = new afn(50, 2);
        y = new afo(51, 1);
        z = new agg(61, 2, afu.i);
        A = new age(62, 2, afu.i);
        ArrayList<aft> arrayList = new ArrayList<aft>();
        for (aft aft2 : b) {
            if (aft2 == null) continue;
            arrayList.add(aft2);
        }
        c = arrayList.toArray(new aft[0]);
    }
}

