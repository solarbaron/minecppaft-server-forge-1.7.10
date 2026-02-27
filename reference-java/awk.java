/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class awk
extends awp {
    private awj[] a;
    private int b;

    public awk(Random random, int n2) {
        this.b = n2;
        this.a = new awj[n2];
        for (int i2 = 0; i2 < n2; ++i2) {
            this.a[i2] = new awj(random);
        }
    }

    public double[] a(double[] dArray, int n2, int n3, int n4, int n5, int n6, int n7, double d2, double d3, double d4) {
        if (dArray == null) {
            dArray = new double[n5 * n6 * n7];
        } else {
            for (int i2 = 0; i2 < dArray.length; ++i2) {
                dArray[i2] = 0.0;
            }
        }
        double d5 = 1.0;
        for (int i3 = 0; i3 < this.b; ++i3) {
            double d6 = (double)n2 * d5 * d2;
            double d7 = (double)n3 * d5 * d3;
            double d8 = (double)n4 * d5 * d4;
            long l2 = qh.d(d6);
            long l3 = qh.d(d8);
            d6 -= (double)l2;
            d8 -= (double)l3;
            this.a[i3].a(dArray, d6 += (double)(l2 %= 0x1000000L), d7, d8 += (double)(l3 %= 0x1000000L), n5, n6, n7, d2 * d5, d3 * d5, d4 * d5, d5);
            d5 /= 2.0;
        }
        return dArray;
    }

    public double[] a(double[] dArray, int n2, int n3, int n4, int n5, double d2, double d3, double d4) {
        return this.a(dArray, n2, 10, n3, n4, 1, n5, d2, 1.0, d3);
    }
}

