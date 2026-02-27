/*
 * Decompiled with CFR 0.152.
 */
public class afq
extends aft {
    private static final String[] E = new String[]{"all", "undead", "arthropods"};
    private static final int[] F = new int[]{1, 5, 5};
    private static final int[] G = new int[]{11, 8, 8};
    private static final int[] H = new int[]{20, 20, 20};
    public final int a;

    public afq(int n2, int n3, int n4) {
        super(n2, n3, afu.g);
        this.a = n4;
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
        return 5;
    }

    @Override
    public float a(int n2, sz sz2) {
        if (this.a == 0) {
            return (float)n2 * 1.25f;
        }
        if (this.a == 1 && sz2 == sz.b) {
            return (float)n2 * 2.5f;
        }
        if (this.a == 2 && sz2 == sz.c) {
            return (float)n2 * 2.5f;
        }
        return 0.0f;
    }

    @Override
    public String a() {
        return "enchantment.damage." + E[this.a];
    }

    @Override
    public boolean a(aft aft2) {
        return !(aft2 instanceof afq);
    }

    @Override
    public boolean a(add add2) {
        if (add2.b() instanceof abf) {
            return true;
        }
        return super.a(add2);
    }

    @Override
    public void a(sv sv2, sa sa2, int n2) {
        if (sa2 instanceof sv) {
            sv sv3 = (sv)sa2;
            if (this.a == 2 && sv3.bd() == sz.c) {
                int n3 = 20 + sv2.aI().nextInt(10 * n2);
                sv3.c(new rw(rv.d.H, n3, 3));
            }
        }
    }
}

