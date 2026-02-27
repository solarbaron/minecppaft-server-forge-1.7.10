/*
 * Decompiled with CFR 0.152.
 */
import java.util.Comparator;

public class vq
implements Comparator {
    private final sa a;

    public vq(sa sa2) {
        this.a = sa2;
    }

    public int a(sa sa2, sa sa3) {
        double d2;
        double d3 = this.a.f(sa2);
        if (d3 < (d2 = this.a.f(sa3))) {
            return -1;
        }
        if (d3 > d2) {
            return 1;
        }
        return 0;
    }

    public /* synthetic */ int compare(Object object, Object object2) {
        return this.a((sa)object, (sa)object2);
    }
}

