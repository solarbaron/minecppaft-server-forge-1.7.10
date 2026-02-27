/*
 * Decompiled with CFR 0.152.
 */
public class afl {
    private String[][] a = new String[][]{{"X", "X", "#"}};
    private Object[][] b = new Object[][]{{ajn.f, ajn.e, ade.j, ade.i, ade.k}, {ade.m, ade.q, ade.l, ade.u, ade.B}};

    public void a(afe afe2) {
        for (int i2 = 0; i2 < this.b[0].length; ++i2) {
            Object object = this.b[0][i2];
            for (int i3 = 0; i3 < this.b.length - 1; ++i3) {
                adb adb2 = (adb)this.b[i3 + 1][i2];
                afe2.a(new add(adb2), this.a[i3], Character.valueOf('#'), ade.y, Character.valueOf('X'), object);
            }
        }
        afe2.a(new add(ade.f, 1), " #X", "# X", " #X", Character.valueOf('X'), ade.F, Character.valueOf('#'), ade.y);
        afe2.a(new add(ade.g, 4), "X", "#", "Y", Character.valueOf('Y'), ade.G, Character.valueOf('X'), ade.ak, Character.valueOf('#'), ade.y);
    }
}

