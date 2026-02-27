/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class vx {
    private static azw a = azw.a(0.0, 0.0, 0.0);

    public static azw a(td td2, int n2, int n3) {
        return vx.c(td2, n2, n3, null);
    }

    public static azw a(td td2, int n2, int n3, azw azw2) {
        vx.a.a = azw2.a - td2.s;
        vx.a.b = azw2.b - td2.t;
        vx.a.c = azw2.c - td2.u;
        return vx.c(td2, n2, n3, a);
    }

    public static azw b(td td2, int n2, int n3, azw azw2) {
        vx.a.a = td2.s - azw2.a;
        vx.a.b = td2.t - azw2.b;
        vx.a.c = td2.u - azw2.c;
        return vx.c(td2, n2, n3, a);
    }

    private static azw c(td td2, int n2, int n3, azw azw2) {
        double d2;
        double d3;
        Random random = td2.aI();
        boolean bl2 = false;
        int n4 = 0;
        int n5 = 0;
        int n6 = 0;
        float f2 = -99999.0f;
        boolean bl3 = td2.bY() ? (d3 = (double)(td2.bV().e(qh.c(td2.s), qh.c(td2.t), qh.c(td2.u)) + 4.0f)) < (d2 = (double)(td2.bW() + (float)n2)) * d2 : false;
        for (int i2 = 0; i2 < 10; ++i2) {
            float f3;
            int n7 = random.nextInt(2 * n2) - n2;
            int n8 = random.nextInt(2 * n3) - n3;
            int n9 = random.nextInt(2 * n2) - n2;
            if (azw2 != null && (double)n7 * azw2.a + (double)n9 * azw2.c < 0.0 || bl3 && !td2.b(n7 += qh.c(td2.s), n8 += qh.c(td2.t), n9 += qh.c(td2.u)) || !((f3 = td2.a(n7, n8, n9)) > f2)) continue;
            f2 = f3;
            n4 = n7;
            n5 = n8;
            n6 = n9;
            bl2 = true;
        }
        if (bl2) {
            return azw.a(n4, n5, n6);
        }
        return null;
    }
}

