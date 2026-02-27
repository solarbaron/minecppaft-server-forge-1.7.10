/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aij
extends ahu {
    private boolean aC;
    private arw aD = new arw();
    private arv aE = new arv(4);

    public aij(int n2, boolean bl2) {
        super(n2);
        this.aC = bl2;
        if (bl2) {
            this.ai = ajn.aE;
        }
        this.at.clear();
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        if (this.aC) {
            int n4;
            int n5;
            int n6;
            for (n6 = 0; n6 < 3; ++n6) {
                n5 = n2 + random.nextInt(16) + 8;
                n4 = n3 + random.nextInt(16) + 8;
                this.aD.a(ahb2, random, n5, ahb2.f(n5, n4), n4);
            }
            for (n6 = 0; n6 < 2; ++n6) {
                n5 = n2 + random.nextInt(16) + 8;
                n4 = n3 + random.nextInt(16) + 8;
                this.aE.a(ahb2, random, n5, ahb2.f(n5, n4), n4);
            }
        }
        super.a(ahb2, random, n2, n3);
    }

    @Override
    public arc a(Random random) {
        return new asn(false);
    }

    @Override
    protected ahu k() {
        ahu ahu2 = new aij(this.ay + 128, true).a(0xD2FFFF, true).a(this.af + " Spikes").c().a(0.0f, 0.5f).a(new ahv(this.am + 0.1f, this.an + 0.1f));
        ahu2.am = this.am + 0.3f;
        ahu2.an = this.an + 0.4f;
        return ahu2;
    }
}

