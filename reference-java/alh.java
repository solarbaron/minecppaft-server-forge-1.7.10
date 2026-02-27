/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class alh
extends aji
implements ajo {
    private static final Logger a = LogManager.getLogger();

    protected alh() {
        super(awt.b);
        this.a(true);
        this.a(abt.b);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        if (ahb2.k(n2, n3 + 1, n4) < 4 && ahb2.a(n2, n3 + 1, n4).k() > 2) {
            ahb2.b(n2, n3, n4, ajn.d);
        } else if (ahb2.k(n2, n3 + 1, n4) >= 9) {
            for (int i2 = 0; i2 < 4; ++i2) {
                int n5 = n2 + random.nextInt(3) - 1;
                int n6 = n3 + random.nextInt(5) - 3;
                int n7 = n4 + random.nextInt(3) - 1;
                aji aji2 = ahb2.a(n5, n6 + 1, n7);
                if (ahb2.a(n5, n6, n7) != ajn.d || ahb2.e(n5, n6, n7) != 0 || ahb2.k(n5, n6 + 1, n7) < 4 || aji2.k() > 2) continue;
                ahb2.b(n5, n6, n7, ajn.c);
            }
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ajn.d.a(0, random, n3);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        return true;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        block0: for (int i2 = 0; i2 < 128; ++i2) {
            int n5 = n2;
            int n6 = n3 + 1;
            int n7 = n4;
            for (int i3 = 0; i3 < i2 / 16; ++i3) {
                if (ahb2.a(n5 += random.nextInt(3) - 1, (n6 += (random.nextInt(3) - 1) * random.nextInt(3) / 2) - 1, n7 += random.nextInt(3) - 1) != ajn.c || ahb2.a(n5, n6, n7).r()) continue block0;
            }
            if (ahb2.a((int)n5, (int)n6, (int)n7).J != awt.a) continue;
            if (random.nextInt(8) != 0) {
                if (!ajn.H.j(ahb2, n5, n6, n7)) continue;
                ahb2.d(n5, n6, n7, ajn.H, 1, 3);
                continue;
            }
            String string = ahb2.a(n5, n7).a(random, n5, n6, n7);
            a.debug("Flower in " + ahb2.a((int)n5, (int)n7).af + ": " + string);
            alc alc2 = alc.e(string);
            if (alc2 == null || !alc2.j(ahb2, n5, n6, n7)) continue;
            int n8 = alc.f(string);
            ahb2.d(n5, n6, n7, alc2, n8, 3);
        }
    }
}

