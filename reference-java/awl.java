/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class awl
extends awp {
    private awo[] a;
    private int b;

    public awl(Random random, int n2) {
        this.b = n2;
        this.a = new awo[n2];
        for (int i2 = 0; i2 < n2; ++i2) {
            this.a[i2] = new awo(random);
        }
    }

    public double a(double d2, double d3) {
        double d4 = 0.0;
        double d5 = 1.0;
        for (int i2 = 0; i2 < this.b; ++i2) {
            d4 += this.a[i2].a(d2 * d5, d3 * d5) / d5;
            d5 /= 2.0;
        }
        return d4;
    }

    public double[] a(double[] dArray, double d2, double d3, int n2, int n3, double d4, double d5, double d6) {
        return this.a(dArray, d2, d3, n2, n3, d4, d5, d6, 0.5);
    }

    public double[] a(double[] dArray, double d2, double d3, int n2, int n3, double d4, double d5, double d6, double d7) {
        if (dArray == null || dArray.length < n2 * n3) {
            dArray = new double[n2 * n3];
        } else {
            for (int i2 = 0; i2 < dArray.length; ++i2) {
                dArray[i2] = 0.0;
            }
        }
        double d8 = 1.0;
        double d9 = 1.0;
        for (int i3 = 0; i3 < this.b; ++i3) {
            this.a[i3].a(dArray, d2, d3, n2, n3, d4 * d9 * d8, d5 * d9 * d8, 0.55 / d8);
            d9 *= d6;
            d8 *= d7;
        }
        return dArray;
    }
}

