/*
 * Decompiled with CFR 0.152.
 */
public class qd {
    private transient qe[] a = new qe[16];
    private transient int b;
    private int c = 12;
    private final float d;
    private volatile transient int e;

    public qd() {
        this.d = 0.75f;
    }

    private static int g(long l2) {
        return qd.a((int)(l2 ^ l2 >>> 32));
    }

    private static int a(int n2) {
        n2 ^= n2 >>> 20 ^ n2 >>> 12;
        return n2 ^ n2 >>> 7 ^ n2 >>> 4;
    }

    private static int a(int n2, int n3) {
        return n2 & n3 - 1;
    }

    public int a() {
        return this.b;
    }

    public Object a(long l2) {
        int n2 = qd.g(l2);
        qe qe2 = this.a[qd.a(n2, this.a.length)];
        while (qe2 != null) {
            if (qe2.a == l2) {
                return qe2.b;
            }
            qe2 = qe2.c;
        }
        return null;
    }

    public boolean b(long l2) {
        return this.c(l2) != null;
    }

    final qe c(long l2) {
        int n2 = qd.g(l2);
        qe qe2 = this.a[qd.a(n2, this.a.length)];
        while (qe2 != null) {
            if (qe2.a == l2) {
                return qe2;
            }
            qe2 = qe2.c;
        }
        return null;
    }

    public void a(long l2, Object object) {
        int n2 = qd.g(l2);
        int n3 = qd.a(n2, this.a.length);
        qe qe2 = this.a[n3];
        while (qe2 != null) {
            if (qe2.a == l2) {
                qe2.b = object;
                return;
            }
            qe2 = qe2.c;
        }
        ++this.e;
        this.a(n2, l2, object, n3);
    }

    private void b(int n2) {
        qe[] qeArray = this.a;
        int n3 = qeArray.length;
        if (n3 == 0x40000000) {
            this.c = Integer.MAX_VALUE;
            return;
        }
        qe[] qeArray2 = new qe[n2];
        this.a(qeArray2);
        this.a = qeArray2;
        this.c = (int)((float)n2 * this.d);
    }

    private void a(qe[] qeArray) {
        qe[] qeArray2 = this.a;
        int n2 = qeArray.length;
        for (int i2 = 0; i2 < qeArray2.length; ++i2) {
            qe qe2;
            qe qe3 = qeArray2[i2];
            if (qe3 == null) continue;
            qeArray2[i2] = null;
            do {
                qe2 = qe3.c;
                int n3 = qd.a(qe3.d, n2);
                qe3.c = qeArray[n3];
                qeArray[n3] = qe3;
            } while ((qe3 = qe2) != null);
        }
    }

    public Object d(long l2) {
        qe qe2 = this.e(l2);
        return qe2 == null ? null : qe2.b;
    }

    final qe e(long l2) {
        qe qe2;
        int n2 = qd.g(l2);
        int n3 = qd.a(n2, this.a.length);
        qe qe3 = qe2 = this.a[n3];
        while (qe3 != null) {
            qe qe4 = qe3.c;
            if (qe3.a == l2) {
                ++this.e;
                --this.b;
                if (qe2 == qe3) {
                    this.a[n3] = qe4;
                } else {
                    qe2.c = qe4;
                }
                return qe3;
            }
            qe2 = qe3;
            qe3 = qe4;
        }
        return qe3;
    }

    private void a(int n2, long l2, Object object, int n3) {
        qe qe2 = this.a[n3];
        this.a[n3] = new qe(n2, l2, object, qe2);
        if (this.b++ >= this.c) {
            this.b(2 * this.a.length);
        }
    }

    static /* synthetic */ int f(long l2) {
        return qd.g(l2);
    }
}

