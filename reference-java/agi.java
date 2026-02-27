/*
 * Decompiled with CFR 0.152.
 */
public class agi
extends aft {
    private static final String[] E = new String[]{"all", "fire", "fall", "explosion", "projectile"};
    private static final int[] F = new int[]{1, 10, 5, 5, 3};
    private static final int[] G = new int[]{11, 8, 6, 8, 6};
    private static final int[] H = new int[]{20, 12, 10, 12, 15};
    public final int a;

    public agi(int n2, int n3, int n4) {
        super(n2, n3, afu.b);
        this.a = n4;
        if (n4 == 2) {
            this.C = afu.c;
        }
    }

    @Override
    public int a(int n2) {
        return F[this.a] + (n2 - 1) * G[this.a];
    }

    @Override
    public int b(int n2) {
        return this.a(n2) + H[this.a];
    }

    @Override
    public int b() {
        return 4;
    }

    @Override
    public int a(int n2, ro ro2) {
        if (ro2.g()) {
            return 0;
        }
        float f2 = (float)(6 + n2 * n2) / 3.0f;
        if (this.a == 0) {
            return qh.d(f2 * 0.75f);
        }
        if (this.a == 1 && ro2.o()) {
            return qh.d(f2 * 1.25f);
        }
        if (this.a == 2 && ro2 == ro.h) {
            return qh.d(f2 * 2.5f);
        }
        if (this.a == 3 && ro2.c()) {
            return qh.d(f2 * 1.5f);
        }
        if (this.a == 4 && ro2.a()) {
            return qh.d(f2 * 1.5f);
        }
        return 0;
    }

    @Override
    public String a() {
        return "enchantment.protect." + E[this.a];
    }

    @Override
    public boolean a(aft aft2) {
        if (aft2 instanceof agi) {
            agi agi2 = (agi)aft2;
            if (agi2.a == this.a) {
                return false;
            }
            return this.a == 2 || agi2.a == 2;
        }
        return super.a(aft2);
    }

    public static int a(sa sa2, int n2) {
        int n3 = afv.a(aft.e.B, sa2.ak());
        if (n3 > 0) {
            n2 -= qh.d((float)n2 * ((float)n3 * 0.15f));
        }
        return n2;
    }

    public static double a(sa sa2, double d2) {
        int n2 = afv.a(aft.g.B, sa2.ak());
        if (n2 > 0) {
            d2 -= (double)qh.c(d2 * (double)((float)n2 * 0.15f));
        }
        return d2;
    }
}

