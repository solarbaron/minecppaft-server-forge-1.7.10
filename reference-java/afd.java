/*
 * Decompiled with CFR 0.152.
 */
public class afd {
    private Object[][] a = new Object[][]{{ajn.R, new add(ade.k, 9)}, {ajn.S, new add(ade.j, 9)}, {ajn.ah, new add(ade.i, 9)}, {ajn.bE, new add(ade.bC, 9)}, {ajn.y, new add(ade.aR, 9, 4)}, {ajn.bX, new add(ade.ax, 9)}, {ajn.ci, new add(ade.h, 9, 0)}, {ajn.cf, new add(ade.O, 9)}};

    public void a(afe afe2) {
        for (int i2 = 0; i2 < this.a.length; ++i2) {
            aji aji2 = (aji)this.a[i2][0];
            add add2 = (add)this.a[i2][1];
            afe2.a(new add(aji2), "###", "###", "###", Character.valueOf('#'), add2);
            afe2.a(add2, "#", Character.valueOf('#'), aji2);
        }
        afe2.a(new add(ade.k), "###", "###", "###", Character.valueOf('#'), ade.bl);
        afe2.a(new add(ade.bl, 9), "#", Character.valueOf('#'), ade.k);
    }
}

