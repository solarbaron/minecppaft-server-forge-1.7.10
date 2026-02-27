/*
 * Decompiled with CFR 0.152.
 */
public abstract class ajc
extends aji
implements akw {
    protected ajc(awt awt2) {
        super(awt2);
        this.A = true;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        super.a(ahb2, n2, n3, n4, aji2, n5);
        ahb2.p(n2, n3, n4);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        super.a(ahb2, n2, n3, n4, n5, n6);
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null) {
            return aor2.c(n5, n6);
        }
        return false;
    }
}

