/*
 * Decompiled with CFR 0.152.
 */
public class aev {
    private String[][] a = new String[][]{{"XXX", "X X"}, {"X X", "XXX", "XXX"}, {"XXX", "X X", "X X"}, {"X X", "X X"}};
    private Object[][] b = new Object[][]{{ade.aA, ajn.ab, ade.j, ade.i, ade.k}, {ade.Q, ade.U, ade.Y, ade.ac, ade.ag}, {ade.R, ade.V, ade.Z, ade.ad, ade.ah}, {ade.S, ade.W, ade.aa, ade.ae, ade.ai}, {ade.T, ade.X, ade.ab, ade.af, ade.aj}};

    public void a(afe afe2) {
        for (int i2 = 0; i2 < this.b[0].length; ++i2) {
            Object object = this.b[0][i2];
            for (int i3 = 0; i3 < this.b.length - 1; ++i3) {
                adb adb2 = (adb)this.b[i3 + 1][i2];
                afe2.a(new add(adb2), this.a[i3], Character.valueOf('X'), object);
            }
        }
    }
}

