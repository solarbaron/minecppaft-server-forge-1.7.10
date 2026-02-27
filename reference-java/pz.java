/*
 * Decompiled with CFR 0.152.
 */
import java.util.HashSet;
import java.util.Set;

public class pz {
    private transient qa[] a;
    private transient int b;
    private int c = 12;
    private final float d;
    private volatile transient int e;
    private Set f = new HashSet();

    public pz() {
        this.d = 0.75f;
        this.a = new qa[16];
    }

    private static int g(int n2) {
        n2 ^= n2 >>> 20 ^ n2 >>> 12;
        return n2 ^ n2 >>> 7 ^ n2 >>> 4;
    }

    private static int a(int n2, int n3) {
        return n2 & n3 - 1;
    }

    public Object a(int n2) {
        int n3 = pz.g(n2);
        qa qa2 = this.a[pz.a(n3, this.a.length)];
        while (qa2 != null) {
            if (qa2.a == n2) {
                return qa2.b;
            }
            qa2 = qa2.c;
        }
        return null;
    }

    public boolean b(int n2) {
        return this.c(n2) != null;
    }

    final qa c(int n2) {
        int n3 = pz.g(n2);
        qa qa2 = this.a[pz.a(n3, this.a.length)];
        while (qa2 != null) {
            if (qa2.a == n2) {
                return qa2;
            }
            qa2 = qa2.c;
        }
        return null;
    }

    public void a(int n2, Object object) {
        this.f.add(n2);
        int n3 = pz.g(n2);
        int n4 = pz.a(n3, this.a.length);
        qa qa2 = this.a[n4];
        while (qa2 != null) {
            if (qa2.a == n2) {
                qa2.b = object;
                return;
            }
            qa2 = qa2.c;
        }
        ++this.e;
        this.a(n3, n2, object, n4);
    }

    private void h(int n2) {
        qa[] qaArray = this.a;
        int n3 = qaArray.length;
        if (n3 == 0x40000000) {
            this.c = Integer.MAX_VALUE;
            return;
        }
        qa[] qaArray2 = new qa[n2];
        this.a(qaArray2);
        this.a = qaArray2;
        this.c = (int)((float)n2 * this.d);
    }

    private void a(qa[] qaArray) {
        qa[] qaArray2 = this.a;
        int n2 = qaArray.length;
        for (int i2 = 0; i2 < qaArray2.length; ++i2) {
            qa qa2;
            qa qa3 = qaArray2[i2];
            if (qa3 == null) continue;
            qaArray2[i2] = null;
            do {
                qa2 = qa3.c;
                int n3 = pz.a(qa3.d, n2);
                qa3.c = qaArray[n3];
                qaArray[n3] = qa3;
            } while ((qa3 = qa2) != null);
        }
    }

    public Object d(int n2) {
        this.f.remove(n2);
        qa qa2 = this.e(n2);
        return qa2 == null ? null : qa2.b;
    }

    final qa e(int n2) {
        qa qa2;
        int n3 = pz.g(n2);
        int n4 = pz.a(n3, this.a.length);
        qa qa3 = qa2 = this.a[n4];
        while (qa3 != null) {
            qa qa4 = qa3.c;
            if (qa3.a == n2) {
                ++this.e;
                --this.b;
                if (qa2 == qa3) {
                    this.a[n4] = qa4;
                } else {
                    qa2.c = qa4;
                }
                return qa3;
            }
            qa2 = qa3;
            qa3 = qa4;
        }
        return qa3;
    }

    public void c() {
        ++this.e;
        qa[] qaArray = this.a;
        for (int i2 = 0; i2 < qaArray.length; ++i2) {
            qaArray[i2] = null;
        }
        this.b = 0;
    }

    private void a(int n2, int n3, Object object, int n4) {
        qa qa2 = this.a[n4];
        this.a[n4] = new qa(n2, n3, object, qa2);
        if (this.b++ >= this.c) {
            this.h(2 * this.a.length);
        }
    }

    static /* synthetic */ int f(int n2) {
        return pz.g(n2);
    }
}

