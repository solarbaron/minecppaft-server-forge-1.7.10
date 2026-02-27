/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

final class kz
extends cm {
    kz() {
    }

    @Override
    public add b(ck ck2, add add2) {
        cr cr2 = akm.b(ck2.h());
        cx cx2 = akm.a(ck2);
        double d2 = cx2.a() + (double)((float)cr2.c() * 0.3f);
        double d3 = cx2.b() + (double)((float)cr2.c() * 0.3f);
        double d4 = cx2.c() + (double)((float)cr2.e() * 0.3f);
        ahb ahb2 = ck2.k();
        Random random = ahb2.s;
        double d5 = random.nextGaussian() * 0.05 + (double)cr2.c();
        double d6 = random.nextGaussian() * 0.05 + (double)cr2.d();
        double d7 = random.nextGaussian() * 0.05 + (double)cr2.e();
        ahb2.d(new zi(ahb2, d2, d3, d4, d5, d6, d7));
        add2.a(1);
        return add2;
    }

    @Override
    protected void a(ck ck2) {
        ck2.k().c(1009, ck2.d(), ck2.e(), ck2.f(), 0);
    }
}

