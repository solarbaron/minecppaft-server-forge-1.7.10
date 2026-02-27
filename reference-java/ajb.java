/*
 * Decompiled with CFR 0.152.
 */
public class ajb
extends akx {
    public static final String[] a = new String[]{"intact", "slightlyDamaged", "veryDamaged"};
    private static final String[] N = new String[]{"anvil_top_damaged_0", "anvil_top_damaged_1", "anvil_top_damaged_2"};

    protected ajb() {
        super(awt.g);
        this.g(0);
        this.a(abt.c);
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3;
        int n6 = ahb2.e(n2, n3, n4) >> 2;
        ++n5;
        if ((n5 %= 4) == 0) {
            ahb2.a(n2, n3, n4, 2 | n6 << 2, 2);
        }
        if (n5 == 1) {
            ahb2.a(n2, n3, n4, 3 | n6 << 2, 2);
        }
        if (n5 == 2) {
            ahb2.a(n2, n3, n4, 0 | n6 << 2, 2);
        }
        if (n5 == 3) {
            ahb2.a(n2, n3, n4, 1 | n6 << 2, 2);
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        yz2.c(n2, n3, n4);
        return true;
    }

    @Override
    public int b() {
        return 35;
    }

    @Override
    public int a(int n2) {
        return n2 >> 2;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4) & 3;
        if (n5 == 3 || n5 == 1) {
            this.a(0.0f, 0.0f, 0.125f, 1.0f, 1.0f, 0.875f);
        } else {
            this.a(0.125f, 0.0f, 0.0f, 0.875f, 1.0f, 1.0f);
        }
    }

    @Override
    protected void a(xj xj2) {
        xj2.a(true);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5) {
        ahb2.c(1022, n2, n3, n4, 0);
    }
}

