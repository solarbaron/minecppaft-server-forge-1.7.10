/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class amf
extends ajr {
    protected amf() {
        this.a(true);
        float f2 = 0.5f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.25f, 0.5f + f2);
        this.a((abt)null);
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2 == ajn.aM;
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return this.a(ahb2.a(n2, n3 - 1, n4));
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4);
        if (n5 < 3 && random.nextInt(10) == 0) {
            ahb2.a(n2, n3, n4, ++n5, 2);
        }
        super.a(ahb2, n2, n3, n4, random);
    }

    @Override
    public int b() {
        return 6;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (ahb2.E) {
            return;
        }
        int n7 = 1;
        if (n5 >= 3) {
            n7 = 2 + ahb2.s.nextInt(3);
            if (n6 > 0) {
                n7 += ahb2.s.nextInt(n6 + 1);
            }
        }
        for (int i2 = 0; i2 < n7; ++i2) {
            this.a(ahb2, n2, n3, n4, new add(ade.bm));
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public int a(Random random) {
        return 0;
    }
}

