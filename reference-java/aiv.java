/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aiv
extends ahu {
    protected aiv(int n2) {
        super(n2);
        this.ar.x = 2;
        this.ar.y = 1;
        this.ar.A = 1;
        this.ar.B = 8;
        this.ar.C = 10;
        this.ar.G = 1;
        this.ar.w = 4;
        this.ar.F = 0;
        this.ar.E = 0;
        this.ar.z = 5;
        this.aq = 14745518;
        this.as.add(new ahx(ym.class, 1, 1, 1));
    }

    @Override
    public arc a(Random random) {
        return this.aB;
    }

    @Override
    public String a(Random random, int n2, int n3, int n4) {
        return alc.a[1];
    }

    @Override
    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        double d3 = ad.a((double)n2 * 0.25, (double)n3 * 0.25);
        if (d3 > 0.0) {
            int n4 = n2 & 0xF;
            int n5 = n3 & 0xF;
            int n6 = ajiArray.length / 256;
            for (int i2 = 255; i2 >= 0; --i2) {
                int n7 = (n5 * 16 + n4) * n6 + i2;
                if (ajiArray[n7] != null && ajiArray[n7].o() == awt.a) continue;
                if (i2 != 62 || ajiArray[n7] == ajn.j) break;
                ajiArray[n7] = ajn.j;
                if (!(d3 < 0.12)) break;
                ajiArray[n7 + 1] = ajn.bi;
                break;
            }
        }
        this.b(ahb2, random, ajiArray, byArray, n2, n3, d2);
    }
}

