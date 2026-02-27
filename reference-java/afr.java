/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class afr
extends aft {
    protected afr(int n2, int n3) {
        super(n2, n3, afu.j);
        this.b("durability");
    }

    @Override
    public int a(int n2) {
        return 5 + (n2 - 1) * 8;
    }

    @Override
    public int b(int n2) {
        return super.a(n2) + 50;
    }

    @Override
    public int b() {
        return 3;
    }

    @Override
    public boolean a(add add2) {
        if (add2.g()) {
            return true;
        }
        return super.a(add2);
    }

    public static boolean a(add add2, int n2, Random random) {
        if (add2.b() instanceof abb && random.nextFloat() < 0.6f) {
            return false;
        }
        return random.nextInt(n2 + 1) > 0;
    }
}

