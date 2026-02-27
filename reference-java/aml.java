/*
 * Decompiled with CFR 0.152.
 */
public class aml
extends alt {
    public static final String[][] N = new String[][]{{"leaves_oak", "leaves_spruce", "leaves_birch", "leaves_jungle"}, {"leaves_oak_opaque", "leaves_spruce_opaque", "leaves_birch_opaque", "leaves_jungle_opaque"}};
    public static final String[] O = new String[]{"oak", "spruce", "birch", "jungle"};

    @Override
    protected void c(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        if ((n5 & 3) == 0 && ahb2.s.nextInt(n6) == 0) {
            this.a(ahb2, n2, n3, n4, new add(ade.e, 1, 0));
        }
    }

    @Override
    protected int b(int n2) {
        int n3 = super.b(n2);
        if ((n2 & 3) == 3) {
            n3 = 40;
        }
        return n3;
    }

    @Override
    public String[] e() {
        return O;
    }
}

