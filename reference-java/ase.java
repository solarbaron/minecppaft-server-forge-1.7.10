/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ase
extends arn {
    private aji a;
    private int b;
    private aji c;

    public ase(aji aji2, int n2) {
        this(aji2, n2, ajn.b);
    }

    public ase(aji aji2, int n2, aji aji3) {
        this.a = aji2;
        this.b = n2;
        this.c = aji3;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        float f2 = random.nextFloat() * (float)Math.PI;
        double d2 = (float)(n2 + 8) + qh.a(f2) * (float)this.b / 8.0f;
        double d3 = (float)(n2 + 8) - qh.a(f2) * (float)this.b / 8.0f;
        double d4 = (float)(n4 + 8) + qh.b(f2) * (float)this.b / 8.0f;
        double d5 = (float)(n4 + 8) - qh.b(f2) * (float)this.b / 8.0f;
        double d6 = n3 + random.nextInt(3) - 2;
        double d7 = n3 + random.nextInt(3) - 2;
        for (int i2 = 0; i2 <= this.b; ++i2) {
            double d8 = d2 + (d3 - d2) * (double)i2 / (double)this.b;
            double d9 = d6 + (d7 - d6) * (double)i2 / (double)this.b;
            double d10 = d4 + (d5 - d4) * (double)i2 / (double)this.b;
            double d11 = random.nextDouble() * (double)this.b / 16.0;
            double d12 = (double)(qh.a((float)i2 * (float)Math.PI / (float)this.b) + 1.0f) * d11 + 1.0;
            double d13 = (double)(qh.a((float)i2 * (float)Math.PI / (float)this.b) + 1.0f) * d11 + 1.0;
            int n5 = qh.c(d8 - d12 / 2.0);
            int n6 = qh.c(d9 - d13 / 2.0);
            int n7 = qh.c(d10 - d12 / 2.0);
            int n8 = qh.c(d8 + d12 / 2.0);
            int n9 = qh.c(d9 + d13 / 2.0);
            int n10 = qh.c(d10 + d12 / 2.0);
            for (int i3 = n5; i3 <= n8; ++i3) {
                double d14 = ((double)i3 + 0.5 - d8) / (d12 / 2.0);
                if (!(d14 * d14 < 1.0)) continue;
                for (int i4 = n6; i4 <= n9; ++i4) {
                    double d15 = ((double)i4 + 0.5 - d9) / (d13 / 2.0);
                    if (!(d14 * d14 + d15 * d15 < 1.0)) continue;
                    for (int i5 = n7; i5 <= n10; ++i5) {
                        double d16 = ((double)i5 + 0.5 - d10) / (d12 / 2.0);
                        if (!(d14 * d14 + d15 * d15 + d16 * d16 < 1.0) || ahb2.a(i3, i4, i5) != this.c) continue;
                        ahb2.d(i3, i4, i5, this.a, 0, 2);
                    }
                }
            }
        }
        return true;
    }
}

