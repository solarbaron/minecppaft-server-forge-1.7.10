/*
 * Decompiled with CFR 0.152.
 */
public class amh
extends alt {
    public static final String[][] N = new String[][]{{"leaves_acacia", "leaves_big_oak"}, {"leaves_acacia_opaque", "leaves_big_oak_opaque"}};
    public static final String[] O = new String[]{"acacia", "big_oak"};

    @Override
    protected void c(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        if ((n5 & 3) == 1 && ahb2.s.nextInt(n6) == 0) {
            this.a(ahb2, n2, n3, n4, new add(ade.e, 1, 0));
        }
    }

    @Override
    public int a(int n2) {
        return super.a(n2) + 4;
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.e(n2, n3, n4) & 3;
    }

    @Override
    public String[] e() {
        return O;
    }
}

