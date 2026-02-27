/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akv
extends ajc {
    protected akv() {
        super(awt.e);
        this.a(abt.c);
        this.a(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 22;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.a(ajn.Z);
    }

    @Override
    public int a(Random random) {
        return 8;
    }

    @Override
    protected boolean E() {
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = 0;
        int n6 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3;
        if (n6 == 0) {
            n5 = 2;
        }
        if (n6 == 1) {
            n5 = 5;
        }
        if (n6 == 2) {
            n5 = 3;
        }
        if (n6 == 3) {
            n5 = 4;
        }
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        aav aav2 = yz2.bS();
        ape ape2 = (ape)ahb2.o(n2, n3, n4);
        if (aav2 == null || ape2 == null) {
            return true;
        }
        if (ahb2.a(n2, n3 + 1, n4).r()) {
            return true;
        }
        if (ahb2.E) {
            return true;
        }
        aav2.a(ape2);
        yz2.a(aav2);
        return true;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new ape();
    }
}

