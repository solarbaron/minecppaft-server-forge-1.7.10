/*
 * Decompiled with CFR 0.152.
 */
import com.google.common.base.Charsets;

public class ou {
    public static char[] a = new char[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    public static String a(byte[] byArray, int n2, int n3) {
        int n4;
        int n5 = n3 - 1;
        int n6 = n4 = n2 > n5 ? n5 : n2;
        while (0 != byArray[n4] && n4 < n5) {
            ++n4;
        }
        return new String(byArray, n2, n4 - n2, Charsets.UTF_8);
    }

    public static int b(byte[] byArray, int n2) {
        return ou.b(byArray, n2, byArray.length);
    }

    public static int b(byte[] byArray, int n2, int n3) {
        if (0 > n3 - n2 - 4) {
            return 0;
        }
        return byArray[n2 + 3] << 24 | (byArray[n2 + 2] & 0xFF) << 16 | (byArray[n2 + 1] & 0xFF) << 8 | byArray[n2] & 0xFF;
    }

    public static int c(byte[] byArray, int n2, int n3) {
        if (0 > n3 - n2 - 4) {
            return 0;
        }
        return byArray[n2] << 24 | (byArray[n2 + 1] & 0xFF) << 16 | (byArray[n2 + 2] & 0xFF) << 8 | byArray[n2 + 3] & 0xFF;
    }

    public static String a(byte by2) {
        return "" + a[(by2 & 0xF0) >>> 4] + a[by2 & 0xF];
    }
}

