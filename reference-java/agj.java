/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class agj
extends aft {
    public agj(int n2, int n3) {
        super(n2, n3, afu.e);
        this.b("thorns");
    }

    @Override
    public int a(int n2) {
        return 10 + 20 * (n2 - 1);
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
        if (add2.b() instanceof abb) {
            return true;
        }
        return super.a(add2);
    }

    @Override
    public void b(sv sv2, sa sa2, int n2) {
        Random random = sv2.aI();
        add add2 = afv.a(aft.k, sv2);
        if (agj.a(n2, random)) {
            sa2.a(ro.a((sa)sv2), (float)agj.b(n2, random));
            sa2.a("damage.thorns", 0.5f, 1.0f);
            if (add2 != null) {
                add2.a(3, sv2);
            }
        } else if (add2 != null) {
            add2.a(1, sv2);
        }
    }

    public static boolean a(int n2, Random random) {
        if (n2 <= 0) {
            return false;
        }
        return random.nextFloat() < 0.15f * (float)n2;
    }

    public static int b(int n2, Random random) {
        if (n2 > 10) {
            return n2 - 10;
        }
        return 1 + random.nextInt(4);
    }
}

