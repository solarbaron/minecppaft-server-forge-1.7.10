/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aki
extends aje {
    public aki() {
        super(true);
        this.a(true);
    }

    @Override
    public int a(ahb ahb2) {
        return 20;
    }

    @Override
    public boolean f() {
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) != 0) {
            return;
        }
        this.a(ahb2, n2, n3, n4, n5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) == 0) {
            return;
        }
        this.a(ahb2, n2, n3, n4, n5);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        return (ahl2.e(n2, n3, n4) & 8) != 0 ? 15 : 0;
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        if ((ahl2.e(n2, n3, n4) & 8) == 0) {
            return 0;
        }
        return n5 == 1 ? 15 : 0;
    }

    private void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        boolean bl2 = (n5 & 8) != 0;
        boolean bl3 = false;
        float f2 = 0.125f;
        List list = ahb2.a(xl.class, azt.a((float)n2 + f2, n3, (float)n4 + f2, (float)(n2 + 1) - f2, (float)(n3 + 1) - f2, (float)(n4 + 1) - f2));
        if (!list.isEmpty()) {
            bl3 = true;
        }
        if (bl3 && !bl2) {
            ahb2.a(n2, n3, n4, n5 | 8, 3);
            ahb2.d(n2, n3, n4, this);
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.c(n2, n3, n4, n2, n3, n4);
        }
        if (!bl3 && bl2) {
            ahb2.a(n2, n3, n4, n5 & 7, 3);
            ahb2.d(n2, n3, n4, this);
            ahb2.d(n2, n3 - 1, n4, this);
            ahb2.c(n2, n3, n4, n2, n3, n4);
        }
        if (bl3) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
        }
        ahb2.f(n2, n3, n4, this);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        this.a(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4));
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        if ((ahb2.e(n2, n3, n4) & 8) > 0) {
            float f2 = 0.125f;
            List list = ahb2.a(xn.class, azt.a((float)n2 + f2, n3, (float)n4 + f2, (float)(n2 + 1) - f2, (float)(n3 + 1) - f2, (float)(n4 + 1) - f2));
            if (list.size() > 0) {
                return ((xn)list.get(0)).e().g();
            }
            List list2 = ahb2.a(xl.class, azt.a((float)n2 + f2, n3, (float)n4 + f2, (float)(n2 + 1) - f2, (float)(n3 + 1) - f2, (float)(n4 + 1) - f2), sj.c);
            if (list2.size() > 0) {
                return zs.b((rb)list2.get(0));
            }
        }
        return 0;
    }
}

