/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aid
extends ahu {
    private arn aC = new ase(ajn.aU, 8);
    private asn aD = new asn(false);
    private int aE = 0;
    private int aF = 1;
    private int aG = 2;
    private int aH = this.aE;

    protected aid(int n2, boolean bl2) {
        super(n2);
        if (bl2) {
            this.ar.x = 3;
            this.aH = this.aF;
        }
    }

    @Override
    public arc a(Random random) {
        if (random.nextInt(3) > 0) {
            return this.aD;
        }
        return super.a(random);
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        int n4;
        int n5;
        int n6;
        super.a(ahb2, random, n2, n3);
        int n7 = 3 + random.nextInt(6);
        for (n6 = 0; n6 < n7; ++n6) {
            int n8;
            n5 = n2 + random.nextInt(16);
            if (ahb2.a(n5, n4 = random.nextInt(28) + 4, n8 = n3 + random.nextInt(16)) != ajn.b) continue;
            ahb2.d(n5, n4, n8, ajn.bA, 0, 2);
        }
        for (n7 = 0; n7 < 7; ++n7) {
            n6 = n2 + random.nextInt(16);
            n5 = random.nextInt(64);
            n4 = n3 + random.nextInt(16);
            this.aC.a(ahb2, random, n6, n5, n4);
        }
    }

    @Override
    public void a(ahb ahb2, Random random, aji[] ajiArray, byte[] byArray, int n2, int n3, double d2) {
        this.ai = ajn.c;
        this.aj = 0;
        this.ak = ajn.d;
        if ((d2 < -1.0 || d2 > 2.0) && this.aH == this.aG) {
            this.ai = ajn.n;
            this.ak = ajn.n;
        } else if (d2 > 1.0 && this.aH != this.aF) {
            this.ai = ajn.b;
            this.ak = ajn.b;
        }
        this.b(ahb2, random, ajiArray, byArray, n2, n3, d2);
    }

    private aid b(ahu ahu2) {
        this.aH = this.aG;
        this.a(ahu2.ag, true);
        this.a(ahu2.af + " M");
        this.a(new ahv(ahu2.am, ahu2.an));
        this.a(ahu2.ao, ahu2.ap);
        return this;
    }

    @Override
    protected ahu k() {
        return new aid(this.ay + 128, false).b(this);
    }
}

