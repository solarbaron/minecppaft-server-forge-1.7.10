/*
 * Decompiled with CFR 0.152.
 */
import java.util.Comparator;

public class ln
implements Comparator {
    private final r a;

    public ln(r r2) {
        this.a = r2;
    }

    public int a(mw mw2, mw mw3) {
        double d2;
        double d3 = mw2.e(this.a.a, this.a.b, this.a.c);
        if (d3 < (d2 = mw3.e(this.a.a, this.a.b, this.a.c))) {
            return -1;
        }
        if (d3 > d2) {
            return 1;
        }
        return 0;
    }

    public /* synthetic */ int compare(Object object, Object object2) {
        return this.a((mw)object, (mw)object2);
    }
}

