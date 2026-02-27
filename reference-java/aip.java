/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aip
extends ahu {
    protected boolean aC;

    protected aip(int n2) {
        super(n2);
        this.a(0.8f, 0.4f);
        this.a(e);
        this.at.add(new ahx(wi.class, 5, 2, 6));
        this.ar.x = -999;
        this.ar.y = 4;
        this.ar.z = 10;
    }

    @Override
    public String a(Random random, int n2, int n3, int n4) {
        double d2 = ad.a((double)n2 / 200.0, (double)n4 / 200.0);
        if (d2 < -0.8) {
            int n5 = random.nextInt(4);
            return alc.a[4 + n5];
        }
        if (random.nextInt(3) > 0) {
            int n6 = random.nextInt(3);
            if (n6 == 0) {
                return alc.a[0];
            }
            if (n6 == 1) {
                return alc.a[3];
            }
            return alc.a[8];
        }
        return alc.b[0];
    }

    @Override
    public void a(ahb ahb2, Random random, int n2, int n3) {
        int n4;
        int n5;
        int n6;
        int n7;
        double d2 = ad.a((double)(n2 + 8) / 200.0, (double)(n3 + 8) / 200.0);
        if (d2 < -0.8) {
            this.ar.y = 15;
            this.ar.z = 5;
        } else {
            this.ar.y = 4;
            this.ar.z = 10;
            ae.a(2);
            for (n7 = 0; n7 < 7; ++n7) {
                n6 = n2 + random.nextInt(16) + 8;
                n5 = n3 + random.nextInt(16) + 8;
                n4 = random.nextInt(ahb2.f(n6, n5) + 32);
                ae.a(ahb2, random, n6, n4, n5);
            }
        }
        if (this.aC) {
            ae.a(0);
            for (n7 = 0; n7 < 10; ++n7) {
                n6 = n2 + random.nextInt(16) + 8;
                n5 = n3 + random.nextInt(16) + 8;
                n4 = random.nextInt(ahb2.f(n6, n5) + 32);
                ae.a(ahb2, random, n6, n4, n5);
            }
        }
        super.a(ahb2, random, n2, n3);
    }

    @Override
    protected ahu k() {
        aip aip2 = new aip(this.ay + 128);
        aip2.a("Sunflower Plains");
        aip2.aC = true;
        aip2.b(9286496);
        aip2.ah = 14273354;
        return aip2;
    }
}

