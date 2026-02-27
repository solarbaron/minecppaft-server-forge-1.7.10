/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

class aig
extends ain {
    final /* synthetic */ aif aC;

    aig(aif aif2, int n2, ahu ahu2) {
        this.aC = aif2;
        super(n2, ahu2);
    }

    @Override
    public arc a(Random random) {
        if (random.nextBoolean()) {
            return aif.aC;
        }
        return aif.aD;
    }
}

