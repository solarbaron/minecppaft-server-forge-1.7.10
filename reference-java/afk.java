/*
 * Decompiled with CFR 0.152.
 */
public class afk {
    private String[][] a = new String[][]{{"XXX", " # ", " # "}, {"X", "#", "#"}, {"XX", "X#", " #"}, {"XX", " #", " #"}};
    private Object[][] b = new Object[][]{{ajn.f, ajn.e, ade.j, ade.i, ade.k}, {ade.o, ade.s, ade.b, ade.w, ade.D}, {ade.n, ade.r, ade.a, ade.v, ade.C}, {ade.p, ade.t, ade.c, ade.x, ade.E}, {ade.I, ade.J, ade.K, ade.L, ade.M}};

    public void a(afe afe2) {
        for (int i2 = 0; i2 < this.b[0].length; ++i2) {
            Object object = this.b[0][i2];
            for (int i3 = 0; i3 < this.b.length - 1; ++i3) {
                adb adb2 = (adb)this.b[i3 + 1][i2];
                afe2.a(new add(adb2), this.a[i3], Character.valueOf('#'), ade.y, Character.valueOf('X'), object);
            }
        }
        afe2.a(new add(ade.aZ), " #", "# ", Character.valueOf('#'), ade.j);
    }
}

