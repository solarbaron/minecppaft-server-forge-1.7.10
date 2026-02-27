/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class qh {
    private static float[] a = new float[65536];
    private static final int[] b;

    public static final float a(float f2) {
        return a[(int)(f2 * 10430.378f) & 0xFFFF];
    }

    public static final float b(float f2) {
        return a[(int)(f2 * 10430.378f + 16384.0f) & 0xFFFF];
    }

    public static final float c(float f2) {
        return (float)Math.sqrt(f2);
    }

    public static final float a(double d2) {
        return (float)Math.sqrt(d2);
    }

    public static int d(float f2) {
        int n2 = (int)f2;
        return f2 < (float)n2 ? n2 - 1 : n2;
    }

    public static int c(double d2) {
        int n2 = (int)d2;
        return d2 < (double)n2 ? n2 - 1 : n2;
    }

    public static long d(double d2) {
        long l2 = (long)d2;
        return d2 < (double)l2 ? l2 - 1L : l2;
    }

    public static float e(float f2) {
        return f2 >= 0.0f ? f2 : -f2;
    }

    public static int a(int n2) {
        return n2 >= 0 ? n2 : -n2;
    }

    public static int f(float f2) {
        int n2 = (int)f2;
        return f2 > (float)n2 ? n2 + 1 : n2;
    }

    public static int f(double d2) {
        int n2 = (int)d2;
        return d2 > (double)n2 ? n2 + 1 : n2;
    }

    public static int a(int n2, int n3, int n4) {
        if (n2 < n3) {
            return n3;
        }
        if (n2 > n4) {
            return n4;
        }
        return n2;
    }

    public static float a(float f2, float f3, float f4) {
        if (f2 < f3) {
            return f3;
        }
        if (f2 > f4) {
            return f4;
        }
        return f2;
    }

    public static double a(double d2, double d3, double d4) {
        if (d2 < d3) {
            return d3;
        }
        if (d2 > d4) {
            return d4;
        }
        return d2;
    }

    public static double b(double d2, double d3, double d4) {
        if (d4 < 0.0) {
            return d2;
        }
        if (d4 > 1.0) {
            return d3;
        }
        return d2 + (d3 - d2) * d4;
    }

    public static double a(double d2, double d3) {
        if (d2 < 0.0) {
            d2 = -d2;
        }
        if (d3 < 0.0) {
            d3 = -d3;
        }
        return d2 > d3 ? d2 : d3;
    }

    public static int a(Random random, int n2, int n3) {
        if (n2 >= n3) {
            return n2;
        }
        return random.nextInt(n3 - n2 + 1) + n2;
    }

    public static float a(Random random, float f2, float f3) {
        if (f2 >= f3) {
            return f2;
        }
        return random.nextFloat() * (f3 - f2) + f2;
    }

    public static double a(Random random, double d2, double d3) {
        if (d2 >= d3) {
            return d2;
        }
        return random.nextDouble() * (d3 - d2) + d2;
    }

    public static double a(long[] lArray) {
        long l2 = 0L;
        for (long l3 : lArray) {
            l2 += l3;
        }
        return (double)l2 / (double)lArray.length;
    }

    public static float g(float f2) {
        if ((f2 %= 360.0f) >= 180.0f) {
            f2 -= 360.0f;
        }
        if (f2 < -180.0f) {
            f2 += 360.0f;
        }
        return f2;
    }

    public static double g(double d2) {
        if ((d2 %= 360.0) >= 180.0) {
            d2 -= 360.0;
        }
        if (d2 < -180.0) {
            d2 += 360.0;
        }
        return d2;
    }

    public static int a(String string, int n2) {
        int n3 = n2;
        try {
            n3 = Integer.parseInt(string);
        }
        catch (Throwable throwable) {
            // empty catch block
        }
        return n3;
    }

    public static int a(String string, int n2, int n3) {
        int n4 = n2;
        try {
            n4 = Integer.parseInt(string);
        }
        catch (Throwable throwable) {
            // empty catch block
        }
        if (n4 < n3) {
            n4 = n3;
        }
        return n4;
    }

    public static double a(String string, double d2) {
        double d3 = d2;
        try {
            d3 = Double.parseDouble(string);
        }
        catch (Throwable throwable) {
            // empty catch block
        }
        return d3;
    }

    public static double a(String string, double d2, double d3) {
        double d4 = d2;
        try {
            d4 = Double.parseDouble(string);
        }
        catch (Throwable throwable) {
            // empty catch block
        }
        if (d4 < d3) {
            d4 = d3;
        }
        return d4;
    }

    static {
        for (int i2 = 0; i2 < 65536; ++i2) {
            qh.a[i2] = (float)Math.sin((double)i2 * Math.PI * 2.0 / 65536.0);
        }
        b = new int[]{0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
    }
}

