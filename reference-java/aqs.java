/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aqs
extends aqx {
    private float[] d = new float[1024];

    protected void a(long l2, int n2, int n3, aji[] ajiArray, double d2, double d3, double d4, float f2, float f3, float f4, int n4, int n5, double d5) {
        int n6;
        Random random = new Random(l2);
        double d6 = n2 * 16 + 8;
        double d7 = n3 * 16 + 8;
        float f5 = 0.0f;
        float f6 = 0.0f;
        if (n5 <= 0) {
            n6 = this.a * 16 - 16;
            n5 = n6 - random.nextInt(n6 / 4);
        }
        n6 = 0;
        if (n4 == -1) {
            n4 = n5 / 2;
            n6 = 1;
        }
        float f7 = 1.0f;
        for (int i2 = 0; i2 < 256; ++i2) {
            if (i2 == 0 || random.nextInt(3) == 0) {
                f7 = 1.0f + random.nextFloat() * random.nextFloat() * 1.0f;
            }
            this.d[i2] = f7 * f7;
        }
        while (n4 < n5) {
            double d8 = 1.5 + (double)(qh.a((float)n4 * (float)Math.PI / (float)n5) * f2 * 1.0f);
            double d9 = d8 * d5;
            d8 *= (double)random.nextFloat() * 0.25 + 0.75;
            d9 *= (double)random.nextFloat() * 0.25 + 0.75;
            float f8 = qh.b(f4);
            float f9 = qh.a(f4);
            d2 += (double)(qh.b(f3) * f8);
            d3 += (double)f9;
            d4 += (double)(qh.a(f3) * f8);
            f4 *= 0.7f;
            f4 += f6 * 0.05f;
            f3 += f5 * 0.05f;
            f6 *= 0.8f;
            f5 *= 0.5f;
            f6 += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 2.0f;
            f5 += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 4.0f;
            if (n6 != 0 || random.nextInt(4) != 0) {
                double d10 = d2 - d6;
                double d11 = d4 - d7;
                double d12 = n5 - n4;
                double d13 = f2 + 2.0f + 16.0f;
                if (d10 * d10 + d11 * d11 - d12 * d12 > d13 * d13) {
                    return;
                }
                if (!(d2 < d6 - 16.0 - d8 * 2.0 || d4 < d7 - 16.0 - d8 * 2.0 || d2 > d6 + 16.0 + d8 * 2.0 || d4 > d7 + 16.0 + d8 * 2.0)) {
                    int n7;
                    int n8;
                    int n9 = qh.c(d2 - d8) - n2 * 16 - 1;
                    int n10 = qh.c(d2 + d8) - n2 * 16 + 1;
                    int n11 = qh.c(d3 - d9) - 1;
                    int n12 = qh.c(d3 + d9) + 1;
                    int n13 = qh.c(d4 - d8) - n3 * 16 - 1;
                    int n14 = qh.c(d4 + d8) - n3 * 16 + 1;
                    if (n9 < 0) {
                        n9 = 0;
                    }
                    if (n10 > 16) {
                        n10 = 16;
                    }
                    if (n11 < 1) {
                        n11 = 1;
                    }
                    if (n12 > 248) {
                        n12 = 248;
                    }
                    if (n13 < 0) {
                        n13 = 0;
                    }
                    if (n14 > 16) {
                        n14 = 16;
                    }
                    boolean bl2 = false;
                    for (n8 = n9; !bl2 && n8 < n10; ++n8) {
                        for (int i3 = n13; !bl2 && i3 < n14; ++i3) {
                            for (int i4 = n12 + 1; !bl2 && i4 >= n11 - 1; --i4) {
                                n7 = (n8 * 16 + i3) * 256 + i4;
                                if (i4 < 0 || i4 >= 256) continue;
                                aji aji2 = ajiArray[n7];
                                if (aji2 == ajn.i || aji2 == ajn.j) {
                                    bl2 = true;
                                }
                                if (i4 == n11 - 1 || n8 == n9 || n8 == n10 - 1 || i3 == n13 || i3 == n14 - 1) continue;
                                i4 = n11;
                            }
                        }
                    }
                    if (!bl2) {
                        for (n8 = n9; n8 < n10; ++n8) {
                            double d14 = ((double)(n8 + n2 * 16) + 0.5 - d2) / d8;
                            for (n7 = n13; n7 < n14; ++n7) {
                                double d15 = ((double)(n7 + n3 * 16) + 0.5 - d4) / d8;
                                int n15 = (n8 * 16 + n7) * 256 + n12;
                                boolean bl3 = false;
                                if (!(d14 * d14 + d15 * d15 < 1.0)) continue;
                                for (int i5 = n12 - 1; i5 >= n11; --i5) {
                                    double d16 = ((double)i5 + 0.5 - d3) / d9;
                                    if ((d14 * d14 + d15 * d15) * (double)this.d[i5] + d16 * d16 / 6.0 < 1.0) {
                                        aji aji3 = ajiArray[n15];
                                        if (aji3 == ajn.c) {
                                            bl3 = true;
                                        }
                                        if (aji3 == ajn.b || aji3 == ajn.d || aji3 == ajn.c) {
                                            if (i5 < 10) {
                                                ajiArray[n15] = ajn.k;
                                            } else {
                                                ajiArray[n15] = null;
                                                if (bl3 && ajiArray[n15 - 1] == ajn.d) {
                                                    ajiArray[n15 - 1] = this.c.a((int)(n8 + n2 * 16), (int)(n7 + n3 * 16)).ai;
                                                }
                                            }
                                        }
                                    }
                                    --n15;
                                }
                            }
                        }
                        if (n6 != 0) break;
                    }
                }
            }
            ++n4;
        }
    }

    @Override
    protected void a(ahb ahb2, int n2, int n3, int n4, int n5, aji[] ajiArray) {
        if (this.b.nextInt(50) != 0) {
            return;
        }
        double d2 = n2 * 16 + this.b.nextInt(16);
        double d3 = this.b.nextInt(this.b.nextInt(40) + 8) + 20;
        double d4 = n3 * 16 + this.b.nextInt(16);
        int n6 = 1;
        for (int i2 = 0; i2 < n6; ++i2) {
            float f2 = this.b.nextFloat() * (float)Math.PI * 2.0f;
            float f3 = (this.b.nextFloat() - 0.5f) * 2.0f / 8.0f;
            float f4 = (this.b.nextFloat() * 2.0f + this.b.nextFloat()) * 2.0f;
            this.a(this.b.nextLong(), n4, n5, ajiArray, d2, d3, d4, f4, f2, f3, 0, 0, 3.0);
        }
    }
}

