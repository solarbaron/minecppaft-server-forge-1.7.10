/*
 * Decompiled with CFR 0.152.
 */
import java.util.Comparator;

class aff
implements Comparator {
    final /* synthetic */ afe a;

    aff(afe afe2) {
        this.a = afe2;
    }

    public int a(afg afg2, afg afg3) {
        if (afg2 instanceof afi && afg3 instanceof afh) {
            return 1;
        }
        if (afg3 instanceof afi && afg2 instanceof afh) {
            return -1;
        }
        if (afg3.a() < afg2.a()) {
            return -1;
        }
        if (afg3.a() > afg2.a()) {
            return 1;
        }
        return 0;
    }

    public /* synthetic */ int compare(Object object, Object object2) {
        return this.a((afg)object, (afg)object2);
    }
}

