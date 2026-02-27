/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

class auw
extends avl {
    private auw() {
    }

    @Override
    public void a(Random random, int n2, int n3, int n4, boolean bl2) {
        if (bl2) {
            this.a = ajn.aV;
            float f2 = random.nextFloat();
            if (f2 < 0.2f) {
                this.b = 2;
            } else if (f2 < 0.5f) {
                this.b = 1;
            } else if (f2 < 0.55f) {
                this.a = ajn.aU;
                this.b = 2;
            } else {
                this.b = 0;
            }
        } else {
            this.a = ajn.a;
            this.b = 0;
        }
    }

    /* synthetic */ auw(auj auj2) {
        this();
    }
}

