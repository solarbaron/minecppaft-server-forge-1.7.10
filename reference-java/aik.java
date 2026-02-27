/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aik
extends ahu {
    private boolean aC;

    public aik(int n2, boolean bl2) {
        super(n2);
        this.aC = bl2;
        this.ar.x = bl2 ? 2 : 50;
        this.ar.z = 25;
        this.ar.y = 4;
        if (!bl2) {
            this.as.add(new ahx(wn.class, 2, 1, 1));
        }
        this.at.add(new ahx(wg.class, 10, 4, 4));
    }

    @Override
    public arc a(Random random) {
        if (random.nextInt(10) == 0) {
            return this.aA;
        }
        if (random.nextInt(2) == 0) {
            return new arp(3, 0);
        }
        if (!this.aC && random.nextInt(3) == 0) {
            return new arz(false, 10, 20, 3, 3);
        }
        return new asq(false, 4 + random.nextInt(7), 3, 3, true);
    }

    @Override
    public arn b(Random random) {
        if (random.nextInt(4) == 0) {
            return new asp(ajn.H, 2);
        }
        return new asp(ajn.H, 1);
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        super.a(ahb2, random, n2, n3);
        int n4 = n2 + random.nextInt(16) + 8;
        int n5 = n3 + random.nextInt(16) + 8;
        int n6 = random.nextInt(ahb2.f(n4, n5) * 2);
        new asc().a(ahb2, random, n4, n6, n5);
        asr asr2 = new asr();
        for (n5 = 0; n5 < 50; ++n5) {
            n6 = n2 + random.nextInt(16) + 8;
            int n7 = 128;
            int n8 = n3 + random.nextInt(16) + 8;
            asr2.a(ahb2, random, n6, n7, n8);
        }
    }
}

