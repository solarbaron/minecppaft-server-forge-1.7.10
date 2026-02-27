/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ais
extends ahu {
    private static final ask aC = new ask(false);

    protected ais(int n2) {
        super(n2);
        this.at.add(new ahx(wi.class, 1, 2, 6));
        this.ar.x = 1;
        this.ar.y = 4;
        this.ar.z = 20;
    }

    @Override
    public arc a(Random random) {
        if (random.nextInt(5) > 0) {
            return aC;
        }
        return this.az;
    }

    @Override
    protected ahu k() {
        ait ait2 = new ait(this.ay + 128, this);
        ait2.ao = (this.ao + 1.0f) * 0.5f;
        ait2.am = this.am * 0.5f + 0.3f;
        ait2.an = this.an * 0.5f + 1.2f;
        return ait2;
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        ae.a(2);
        for (int i2 = 0; i2 < 7; ++i2) {
            int n4 = n2 + random.nextInt(16) + 8;
            int n5 = n3 + random.nextInt(16) + 8;
            int n6 = random.nextInt(ahb2.f(n4, n5) + 32);
            ae.a(ahb2, random, n4, n6, n5);
        }
        super.a(ahb2, random, n2, n3);
    }
}

