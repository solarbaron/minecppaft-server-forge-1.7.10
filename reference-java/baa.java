/*
 * Decompiled with CFR 0.152.
 */
import java.util.Comparator;

final class baa
implements Comparator {
    baa() {
    }

    public int a(azz azz2, azz azz3) {
        if (azz2.c() > azz3.c()) {
            return 1;
        }
        if (azz2.c() < azz3.c()) {
            return -1;
        }
        return 0;
    }

    public /* synthetic */ int compare(Object object, Object object2) {
        return this.a((azz)object, (azz)object2);
    }
}

