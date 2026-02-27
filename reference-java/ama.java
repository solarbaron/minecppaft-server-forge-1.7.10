/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ama
extends ajc {
    protected ama() {
        super(awt.e);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apj();
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, n6);
        int n7 = 15 + ahb2.s.nextInt(15) + ahb2.s.nextInt(15);
        this.c(ahb2, n2, n3, n4, n7);
    }

    @Override
    public boolean c() {
        return false;
    }
}

