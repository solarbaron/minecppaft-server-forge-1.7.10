/*
 * Decompiled with CFR 0.152.
 */
public class alc
extends ajr {
    private static final String[][] M = new String[][]{{"flower_dandelion"}, {"flower_rose", "flower_blue_orchid", "flower_allium", "flower_houstonia", "flower_tulip_red", "flower_tulip_orange", "flower_tulip_white", "flower_tulip_pink", "flower_oxeye_daisy"}};
    public static final String[] a = new String[]{"poppy", "blueOrchid", "allium", "houstonia", "tulipRed", "tulipOrange", "tulipWhite", "tulipPink", "oxeyeDaisy"};
    public static final String[] b = new String[]{"dandelion"};
    private int O;

    protected alc(int n2) {
        super(awt.k);
        this.O = n2;
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    public static alc e(String string) {
        for (String string2 : b) {
            if (!string2.equals(string)) continue;
            return ajn.N;
        }
        for (String string2 : a) {
            if (!string2.equals(string)) continue;
            return ajn.O;
        }
        return null;
    }

    public static int f(String string) {
        int n2;
        for (n2 = 0; n2 < b.length; ++n2) {
            if (!b[n2].equals(string)) continue;
            return n2;
        }
        for (n2 = 0; n2 < a.length; ++n2) {
            if (!a[n2].equals(string)) continue;
            return n2;
        }
        return 0;
    }
}

