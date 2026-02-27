/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anf
extends akj {
    public static final double[] b = new double[]{-0.0625, 0.0625, 0.1875, 0.3125};
    private static final int[] M = new int[]{1, 2, 3, 4};

    protected anf(boolean bl2) {
        super(bl2);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = (n6 & 0xC) >> 2;
        n7 = n7 + 1 << 2 & 0xC;
        ahb2.a(n2, n3, n4, n7 | n6 & 3, 3);
        return true;
    }

    @Override
    protected int b(int n2) {
        return M[(n2 & 0xC) >> 2] * 2;
    }

    @Override
    protected akj e() {
        return ajn.aS;
    }

    @Override
    protected akj i() {
        return ajn.aR;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.aW;
    }

    @Override
    public int b() {
        return 15;
    }

    @Override
    public boolean g(ahl ahl2, int n2, int n3, int n4, int n5) {
        return this.h(ahl2, n2, n3, n4, n5) > 0;
    }

    @Override
    protected boolean a(aji aji2) {
        return anf.d(aji2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        super.a(ahb2, n2, n3, n4, aji2, n5);
        this.e(ahb2, n2, n3, n4);
    }
}

