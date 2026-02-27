/*
 * Decompiled with CFR 0.152.
 */
public class amj
extends ajc {
    public amj() {
        super(awt.d);
        this.a(abt.d);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        boolean bl2 = ahb2.v(n2, n3, n4);
        apl apl2 = (apl)ahb2.o(n2, n3, n4);
        if (apl2 != null && apl2.i != bl2) {
            if (bl2) {
                apl2.a(ahb2, n2, n3, n4);
            }
            apl2.i = bl2;
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        apl apl2 = (apl)ahb2.o(n2, n3, n4);
        if (apl2 != null) {
            apl2.a();
            apl2.a(ahb2, n2, n3, n4);
        }
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        if (ahb2.E) {
            return;
        }
        apl apl2 = (apl)ahb2.o(n2, n3, n4);
        if (apl2 != null) {
            apl2.a(ahb2, n2, n3, n4);
        }
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apl();
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        float f2 = (float)Math.pow(2.0, (double)(n6 - 12) / 12.0);
        String string = "harp";
        if (n5 == 1) {
            string = "bd";
        }
        if (n5 == 2) {
            string = "snare";
        }
        if (n5 == 3) {
            string = "hat";
        }
        if (n5 == 4) {
            string = "bassattack";
        }
        ahb2.a((double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, "note." + string, 3.0f, f2);
        ahb2.a("note", (double)n2 + 0.5, (double)n3 + 1.2, (double)n4 + 0.5, (double)n6 / 24.0, 0.0, 0.0);
        return true;
    }
}

