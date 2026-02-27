/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aif
extends ahu {
    private int aF;
    protected static final are aC = new are(false, true);
    protected static final are aD = new are(false, false);
    protected static final asi aE = new asi(false);

    public aif(int n2, int n3) {
        super(n2);
        this.aF = n3;
        this.ar.x = 10;
        this.ar.z = 2;
        if (this.aF == 1) {
            this.ar.x = 6;
            this.ar.y = 100;
            this.ar.z = 1;
        }
        this.a(5159473);
        this.a(0.7f, 0.8f);
        if (this.aF == 2) {
            this.ah = 353825;
            this.ag = 3175492;
            this.a(0.6f, 0.6f);
        }
        if (this.aF == 0) {
            this.at.add(new ahx(wv.class, 5, 4, 4));
        }
        if (this.aF == 3) {
            this.ar.x = -999;
        }
    }

    @Override
    protected ahu a(int n2, boolean bl2) {
        if (this.aF == 2) {
            this.ah = 353825;
            this.ag = n2;
            if (bl2) {
                this.ah = (this.ah & 0xFEFEFE) >> 1;
            }
            return this;
        }
        return super.a(n2, bl2);
    }

    @Override
    public arc a(Random random) {
        if (this.aF == 3 && random.nextInt(3) > 0) {
            return aE;
        }
        if (this.aF == 2 || random.nextInt(5) == 0) {
            return aD;
        }
        return this.az;
    }

    @Override
    public String a(Random random, int n2, int n3, int n4) {
        if (this.aF == 1) {
            double d2 = qh.a((1.0 + ad.a((double)n2 / 48.0, (double)n4 / 48.0)) / 2.0, 0.0, 0.9999);
            int n5 = (int)(d2 * (double)alc.a.length);
            if (n5 == 1) {
                n5 = 0;
            }
            return alc.a[n5];
        }
        return super.a(random, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        int n4;
        int n5;
        int n6;
        int n7;
        int n8;
        if (this.aF == 3) {
            for (n8 = 0; n8 < 4; ++n8) {
                for (n7 = 0; n7 < 4; ++n7) {
                    arn arn2;
                    n6 = n2 + n8 * 4 + 1 + 8 + random.nextInt(3);
                    n5 = n3 + n7 * 4 + 1 + 8 + random.nextInt(3);
                    n4 = ahb2.f(n6, n5);
                    if (random.nextInt(20) == 0) {
                        arn2 = new aru();
                        ((aru)arn2).a(ahb2, random, n6, n4, n5);
                        continue;
                    }
                    arn2 = this.a(random);
                    arn2.a(1.0, 1.0, 1.0);
                    if (!arn2.a(ahb2, random, n6, n4, n5)) continue;
                    ((arc)arn2).b(ahb2, random, n6, n4, n5);
                }
            }
        }
        n8 = random.nextInt(5) - 3;
        if (this.aF == 1) {
            n8 += 2;
        }
        for (n7 = 0; n7 < n8; ++n7) {
            int n9;
            int n10;
            n6 = random.nextInt(3);
            if (n6 == 0) {
                ae.a(1);
            } else if (n6 == 1) {
                ae.a(4);
            } else if (n6 == 2) {
                ae.a(5);
            }
            for (n5 = 0; n5 < 5 && !ae.a(ahb2, random, n4 = n2 + random.nextInt(16) + 8, n10 = random.nextInt(ahb2.f(n4, n9 = n3 + random.nextInt(16) + 8) + 32), n9); ++n5) {
            }
        }
        super.a(ahb2, random, n2, n3);
    }

    @Override
    protected ahu k() {
        if (this.ay == ahu.s.ay) {
            aif aif2 = new aif(this.ay + 128, 1);
            aif2.a(new ahv(this.am, this.an + 0.2f));
            aif2.a("Flower Forest");
            aif2.a(6976549, true);
            aif2.a(8233509);
            return aif2;
        }
        if (this.ay == ahu.P.ay || this.ay == ahu.Q.ay) {
            return new aig(this, this.ay + 128, this);
        }
        return new aih(this, this.ay + 128, this);
    }
}

