/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class anm
extends aji {
    protected anm() {
        super(awt.z);
        this.a(true);
        this.a(abt.b);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ay;
    }

    @Override
    public int a(Random random) {
        return 4;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.b(ahn.b, n2, n3, n4) > 11) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
        }
    }
}

