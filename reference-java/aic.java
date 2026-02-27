/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aic
extends ahu {
    public aic(int n2) {
        super(n2);
        this.at.clear();
        this.ai = ajn.m;
        this.ak = ajn.m;
        this.ar.x = -999;
        this.ar.A = 2;
        this.ar.C = 50;
        this.ar.D = 10;
        this.at.clear();
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        super.a(ahb2, random, n2, n3);
        if (random.nextInt(1000) == 0) {
            int n4 = n2 + random.nextInt(16) + 8;
            int n5 = n3 + random.nextInt(16) + 8;
            arl arl2 = new arl();
            ((arn)arl2).a(ahb2, random, n4, ahb2.f(n4, n5) + 1, n5);
        }
    }
}

