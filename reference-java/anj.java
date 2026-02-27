/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anj
extends ajr
implements ajo {
    public static final String[] a = new String[]{"oak", "spruce", "birch", "jungle", "acacia", "roofed_oak"};
    private static final rf[] b = new rf[a.length];

    protected anj() {
        float f2 = 0.4f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f2 * 2.0f, 0.5f + f2);
        this.a(abt.c);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        super.a(ahb2, n2, n3, n4, random);
        if (ahb2.k(n2, n3 + 1, n4) >= 9 && random.nextInt(7) == 0) {
            this.c(ahb2, n2, n3, n4, random);
        }
    }

    public void c(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) == 0) {
            ahb2.a(n2, n3, n4, n5 | 8, 4);
        } else {
            this.d(ahb2, n2, n3, n4, random);
        }
    }

    public void d(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5 = ahb2.e(n2, n3, n4) & 7;
        arc arc2 = random.nextInt(10) == 0 ? new ard(true) : new asq(true);
        int n6 = 0;
        int n7 = 0;
        boolean bl2 = false;
        switch (n5) {
            case 1: {
                block7: for (n6 = 0; n6 >= -1; --n6) {
                    for (n7 = 0; n7 >= -1; --n7) {
                        if (!this.a(ahb2, n2 + n6, n3, n4 + n7, 1) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7, 1) || !this.a(ahb2, n2 + n6, n3, n4 + n7 + 1, 1) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7 + 1, 1)) continue;
                        arc2 = new asa(false, random.nextBoolean());
                        bl2 = true;
                        break block7;
                    }
                }
                if (bl2) break;
                n7 = 0;
                n6 = 0;
                arc2 = new asn(true);
                break;
            }
            case 2: {
                arc2 = new are(true, false);
                break;
            }
            case 3: {
                block9: for (n6 = 0; n6 >= -1; --n6) {
                    for (n7 = 0; n7 >= -1; --n7) {
                        if (!this.a(ahb2, n2 + n6, n3, n4 + n7, 3) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7, 3) || !this.a(ahb2, n2 + n6, n3, n4 + n7 + 1, 3) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7 + 1, 3)) continue;
                        arc2 = new arz(true, 10, 20, 3, 3);
                        bl2 = true;
                        break block9;
                    }
                }
                if (bl2) break;
                n7 = 0;
                n6 = 0;
                arc2 = new asq(true, 4 + random.nextInt(7), 3, 3, false);
                break;
            }
            case 4: {
                arc2 = new ask(true);
                break;
            }
            case 5: {
                block11: for (n6 = 0; n6 >= -1; --n6) {
                    for (n7 = 0; n7 >= -1; --n7) {
                        if (!this.a(ahb2, n2 + n6, n3, n4 + n7, 5) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7, 5) || !this.a(ahb2, n2 + n6, n3, n4 + n7 + 1, 5) || !this.a(ahb2, n2 + n6 + 1, n3, n4 + n7 + 1, 5)) continue;
                        arc2 = new asi(true);
                        bl2 = true;
                        break block11;
                    }
                }
                if (bl2) break;
                return;
            }
        }
        aji aji2 = ajn.a;
        if (bl2) {
            ahb2.d(n2 + n6, n3, n4 + n7, aji2, 0, 4);
            ahb2.d(n2 + n6 + 1, n3, n4 + n7, aji2, 0, 4);
            ahb2.d(n2 + n6, n3, n4 + n7 + 1, aji2, 0, 4);
            ahb2.d(n2 + n6 + 1, n3, n4 + n7 + 1, aji2, 0, 4);
        } else {
            ahb2.d(n2, n3, n4, aji2, 0, 4);
        }
        if (!((arn)arc2).a(ahb2, random, n2 + n6, n3, n4 + n7)) {
            if (bl2) {
                ahb2.d(n2 + n6, n3, n4 + n7, this, n5, 4);
                ahb2.d(n2 + n6 + 1, n3, n4 + n7, this, n5, 4);
                ahb2.d(n2 + n6, n3, n4 + n7 + 1, this, n5, 4);
                ahb2.d(n2 + n6 + 1, n3, n4 + n7 + 1, this, n5, 4);
            } else {
                ahb2.d(n2, n3, n4, this, n5, 4);
            }
        }
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5) {
        return ahb2.a(n2, n3, n4) == this && (ahb2.e(n2, n3, n4) & 7) == n5;
    }

    @Override
    public int a(int n2) {
        return qh.a(n2 & 7, 0, 5);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        return true;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return (double)ahb2.s.nextFloat() < 0.45;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        this.c(ahb2, n2, n3, n4, random);
    }
}

