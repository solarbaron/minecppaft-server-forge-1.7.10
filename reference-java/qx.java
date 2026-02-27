/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class qx
extends qw {
    private add b;
    private int c;
    private int d;

    public qx(adb adb2, int n2, int n3, int n4, int n5) {
        super(n5);
        this.b = new add(adb2, 1, n2);
        this.c = n3;
        this.d = n4;
    }

    public qx(add add2, int n2, int n3, int n4) {
        super(n4);
        this.b = add2;
        this.c = n2;
        this.d = n3;
    }

    public static void a(Random random, qx[] qxArray, rb rb2, int n2) {
        for (int i2 = 0; i2 < n2; ++i2) {
            qx qx2 = (qx)qv.a(random, qxArray);
            int n3 = qx2.c + random.nextInt(qx2.d - qx2.c + 1);
            if (qx2.b.e() >= n3) {
                add add2 = qx2.b.m();
                add2.b = n3;
                rb2.a(random.nextInt(rb2.a()), add2);
                continue;
            }
            for (int i3 = 0; i3 < n3; ++i3) {
                add add3 = qx2.b.m();
                add3.b = 1;
                rb2.a(random.nextInt(rb2.a()), add3);
            }
        }
    }

    public static void a(Random random, qx[] qxArray, apb apb2, int n2) {
        for (int i2 = 0; i2 < n2; ++i2) {
            qx qx2 = (qx)qv.a(random, qxArray);
            int n3 = qx2.c + random.nextInt(qx2.d - qx2.c + 1);
            if (qx2.b.e() >= n3) {
                add add2 = qx2.b.m();
                add2.b = n3;
                apb2.a(random.nextInt(apb2.a()), add2);
                continue;
            }
            for (int i3 = 0; i3 < n3; ++i3) {
                add add3 = qx2.b.m();
                add3.b = 1;
                apb2.a(random.nextInt(apb2.a()), add3);
            }
        }
    }

    public static qx[] a(qx[] qxArray, qx ... qxArray2) {
        qx[] qxArray3 = new qx[qxArray.length + qxArray2.length];
        int n2 = 0;
        for (int i2 = 0; i2 < qxArray.length; ++i2) {
            qxArray3[n2++] = qxArray[i2];
        }
        for (qx qx2 : qxArray2) {
            qxArray3[n2++] = qx2;
        }
        return qxArray3;
    }
}

