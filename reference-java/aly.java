/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aly
extends aji {
    protected aly() {
        super(awt.C);
        this.a(abt.b);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ba;
    }

    @Override
    public int a(Random random) {
        return 3 + random.nextInt(5);
    }

    @Override
    public int a(int n2, Random random) {
        int n3 = this.a(random) + random.nextInt(1 + n2);
        if (n3 > 9) {
            n3 = 9;
        }
        return n3;
    }
}

