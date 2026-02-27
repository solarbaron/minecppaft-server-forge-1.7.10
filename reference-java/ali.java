/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ali
extends akx {
    @Override
    public adb a(int n2, Random random, int n3) {
        if (n3 > 3) {
            n3 = 3;
        }
        if (random.nextInt(10 - n3 * 3) == 0) {
            return ade.ak;
        }
        return adb.a(this);
    }
}

