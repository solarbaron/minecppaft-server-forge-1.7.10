/*
 * Decompiled with CFR 0.152.
 */
import java.util.Arrays;
import java.util.Random;

public class ail
extends ahu {
    private byte[] aC;
    private long aD;
    private awl aE;
    private awl aF;
    private awl aG;
    private boolean aH;
    private boolean aI;

    public ail(int n2, boolean bl2, boolean bl3) {
        super(n2);
        this.aH = bl2;
        this.aI = bl3;
        this.b();
        this.a(2.0f, 0.0f);
        this.at.clear();
        this.ai = ajn.m;
        this.aj = 1;
        this.ak = ajn.ce;
        this.ar.x = -999;
        this.ar.A = 20;
        this.ar.C = 3;
        this.ar.D = 5;
        this.ar.y = 0;
        this.at.clear();
        if (bl3) {
            this.ar.x = 5;
        }
    }

    @Override
    public arc a(Random random) {
        return this.az;
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        super.a(ahb2, random, n2, n3);
    }

    @Override
    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        int n4;
        int n5;
        if (this.aC == null || this.aD != ahb2.H()) {
            this.a(ahb2.H());
        }
        if (this.aE == null || this.aF == null || this.aD != ahb2.H()) {
            Random random2 = new Random(this.aD);
            this.aE = new awl(random2, 4);
            this.aF = new awl(random2, 1);
        }
        this.aD = ahb2.H();
        double d3 = 0.0;
        if (this.aH) {
            n5 = (n2 & 0xFFFFFFF0) + (n3 & 0xF);
            n4 = (n3 & 0xFFFFFFF0) + (n2 & 0xF);
            double d4 = Math.min(Math.abs(d2), this.aE.a((double)n5 * 0.25, (double)n4 * 0.25));
            if (d4 > 0.0) {
                d3 = d4 * d4 * 2.5;
                double d5 = 0.001953125;
                double d6 = Math.abs(this.aF.a((double)n5 * d5, (double)n4 * d5));
                double d7 = Math.ceil(d6 * 50.0) + 14.0;
                if (d3 > d7) {
                    d3 = d7;
                }
                d3 += 64.0;
            }
        }
        n5 = n2 & 0xF;
        n4 = n3 & 0xF;
        int n6 = 63;
        aji aji2 = ajn.ce;
        aji aji3 = this.ak;
        int n7 = (int)(d2 / 3.0 + 3.0 + random.nextDouble() * 0.25);
        boolean bl2 = Math.cos(d2 / 3.0 * Math.PI) > 0.0;
        int n8 = -1;
        boolean bl3 = false;
        int n9 = ajiArray.length / 256;
        for (int i2 = 255; i2 >= 0; --i2) {
            int n10;
            int n11 = (n4 * 16 + n5) * n9 + i2;
            if ((ajiArray[n11] == null || ajiArray[n11].o() == awt.a) && i2 < (int)d3) {
                ajiArray[n11] = ajn.b;
            }
            if (i2 <= 0 + random.nextInt(5)) {
                ajiArray[n11] = ajn.h;
                continue;
            }
            aji aji4 = ajiArray[n11];
            if (aji4 == null || aji4.o() == awt.a) {
                n8 = -1;
                continue;
            }
            if (aji4 != ajn.b) continue;
            if (n8 == -1) {
                bl3 = false;
                if (n7 <= 0) {
                    aji2 = null;
                    aji3 = ajn.b;
                } else if (i2 >= 59 && i2 <= 64) {
                    aji2 = ajn.ce;
                    aji3 = this.ak;
                }
                if (i2 < 63 && (aji2 == null || aji2.o() == awt.a)) {
                    aji2 = ajn.j;
                }
                n8 = n7 + Math.max(0, i2 - 63);
                if (i2 >= 62) {
                    if (this.aI && i2 > 86 + n7 * 2) {
                        if (bl2) {
                            ajiArray[n11] = ajn.d;
                            byArray[n11] = 1;
                            continue;
                        }
                        ajiArray[n11] = ajn.c;
                        continue;
                    }
                    if (i2 > 66 + n7) {
                        n10 = 16;
                        if (i2 < 64 || i2 > 127) {
                            n10 = 1;
                        } else if (!bl2) {
                            n10 = this.d(n2, i2, n3);
                        }
                        if (n10 < 16) {
                            ajiArray[n11] = ajn.ce;
                            byArray[n11] = (byte)n10;
                            continue;
                        }
                        ajiArray[n11] = ajn.ch;
                        continue;
                    }
                    ajiArray[n11] = this.ai;
                    byArray[n11] = (byte)this.aj;
                    bl3 = true;
                    continue;
                }
                ajiArray[n11] = aji3;
                if (aji3 != ajn.ce) continue;
                byArray[n11] = 1;
                continue;
            }
            if (n8 <= 0) continue;
            --n8;
            if (bl3) {
                ajiArray[n11] = ajn.ce;
                byArray[n11] = 1;
                continue;
            }
            n10 = this.d(n2, i2, n3);
            if (n10 < 16) {
                ajiArray[n11] = ajn.ce;
                byArray[n11] = n10;
                continue;
            }
            ajiArray[n11] = ajn.ch;
        }
    }

    private void a(long l2) {
        int n2;
        int n3;
        int n4;
        int n5;
        int n6;
        int n7;
        int n8;
        this.aC = new byte[64];
        Arrays.fill(this.aC, (byte)16);
        Random random = new Random(l2);
        this.aG = new awl(random, 1);
        for (n8 = 0; n8 < 64; ++n8) {
            if ((n8 += random.nextInt(5) + 1) >= 64) continue;
            this.aC[n8] = 1;
        }
        n8 = random.nextInt(4) + 2;
        for (n7 = 0; n7 < n8; ++n7) {
            n6 = random.nextInt(3) + 1;
            n5 = random.nextInt(64);
            for (n4 = 0; n5 + n4 < 64 && n4 < n6; ++n4) {
                this.aC[n5 + n4] = 4;
            }
        }
        n7 = random.nextInt(4) + 2;
        for (n6 = 0; n6 < n7; ++n6) {
            n5 = random.nextInt(3) + 2;
            n4 = random.nextInt(64);
            for (n3 = 0; n4 + n3 < 64 && n3 < n5; ++n3) {
                this.aC[n4 + n3] = 12;
            }
        }
        n6 = random.nextInt(4) + 2;
        for (n5 = 0; n5 < n6; ++n5) {
            n4 = random.nextInt(3) + 1;
            n3 = random.nextInt(64);
            for (n2 = 0; n3 + n2 < 64 && n2 < n4; ++n2) {
                this.aC[n3 + n2] = 14;
            }
        }
        n5 = random.nextInt(3) + 3;
        n4 = 0;
        for (n3 = 0; n3 < n5; ++n3) {
            n2 = 1;
            n4 += random.nextInt(16) + 4;
            for (int i2 = 0; n4 + i2 < 64 && i2 < n2; ++i2) {
                this.aC[n4 + i2] = 0;
                if (n4 + i2 > 1 && random.nextBoolean()) {
                    this.aC[n4 + i2 - 1] = 8;
                }
                if (n4 + i2 >= 63 || !random.nextBoolean()) continue;
                this.aC[n4 + i2 + 1] = 8;
            }
        }
    }

    private byte d(int n2, int n3, int n4) {
        int n5 = (int)Math.round(this.aG.a((double)n2 * 1.0 / 512.0, (double)n2 * 1.0 / 512.0) * 2.0);
        return this.aC[(n3 + n5 + 64) % 64];
    }

    @Override
    protected ahu k() {
        boolean bl2 = this.ay == ahu.Z.ay;
        ail ail2 = new ail(this.ay + 128, bl2, this.aI);
        if (!bl2) {
            ail2.a(g);
            ail2.a(this.af + " M");
        } else {
            ail2.a(this.af + " (Bryce)");
        }
        ail2.a(this.ag, true);
        return ail2;
    }
}

