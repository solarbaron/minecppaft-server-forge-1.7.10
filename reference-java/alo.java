/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class alo
extends aji {
    private static final String[] a = new String[]{"skin_brown", "skin_red"};
    private final int b;

    public alo(awt awt2, int n2) {
        super(awt2);
        this.b = n2;
    }

    @Override
    public int a(Random random) {
        int n2 = random.nextInt(10) - 7;
        if (n2 < 0) {
            n2 = 0;
        }
        return n2;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return adb.d(aji.b(ajn.P) + this.b);
    }
}

