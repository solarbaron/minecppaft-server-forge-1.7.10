/*
 * Decompiled with CFR 0.152.
 */
public class p {
    public static final int[] a = new int[]{0, -1, 0, 1};
    public static final int[] b = new int[]{1, 0, -1, 0};
    public static final String[] c = new String[]{"SOUTH", "WEST", "NORTH", "EAST"};
    public static final int[] d = new int[]{3, 4, 2, 5};
    public static final int[] e = new int[]{-1, -1, 2, 0, 1, 3};
    public static final int[] f = new int[]{2, 3, 0, 1};
    public static final int[] g = new int[]{1, 2, 3, 0};
    public static final int[] h = new int[]{3, 0, 1, 2};
    public static final int[][] i = new int[][]{{1, 0, 3, 2, 5, 4}, {1, 0, 5, 4, 2, 3}, {1, 0, 2, 3, 4, 5}, {1, 0, 4, 5, 3, 2}};

    public static int a(double d2, double d3) {
        if (qh.e((float)d2) > qh.e((float)d3)) {
            if (d2 > 0.0) {
                return 1;
            }
            return 3;
        }
        if (d3 > 0.0) {
            return 2;
        }
        return 0;
    }
}

