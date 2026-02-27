/*
 * Decompiled with CFR 0.152.
 */
import java.util.Collection;
import java.util.Random;

public class qv {
    public static int a(Collection collection) {
        int n2 = 0;
        for (qw qw2 : collection) {
            n2 += qw2.a;
        }
        return n2;
    }

    public static qw a(Random random, Collection collection, int n2) {
        if (n2 <= 0) {
            throw new IllegalArgumentException();
        }
        int n3 = random.nextInt(n2);
        for (qw qw2 : collection) {
            if ((n3 -= qw2.a) >= 0) continue;
            return qw2;
        }
        return null;
    }

    public static qw a(Random random, Collection collection) {
        return qv.a(random, collection, qv.a(collection));
    }

    public static int a(qw[] qwArray) {
        int n2 = 0;
        for (qw qw2 : qwArray) {
            n2 += qw2.a;
        }
        return n2;
    }

    public static qw a(Random random, qw[] qwArray, int n2) {
        if (n2 <= 0) {
            throw new IllegalArgumentException();
        }
        int n3 = random.nextInt(n2);
        for (qw qw2 : qwArray) {
            if ((n3 -= qw2.a) >= 0) continue;
            return qw2;
        }
        return null;
    }

    public static qw a(Random random, qw[] qwArray) {
        return qv.a(random, qwArray, qv.a(qwArray));
    }
}

