/*
 * Decompiled with CFR 0.152.
 */
public class aks
extends ajc {
    protected aks() {
        super(awt.e);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
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
    public aor a(ahb ahb2, int n2) {
        return new apd();
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        apd apd2 = (apd)ahb2.o(n2, n3, n4);
        yz2.a(n2, n3, n4, apd2.b() ? apd2.a() : null);
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        super.a(ahb2, n2, n3, n4, sv2, add2);
        if (add2.u()) {
            ((apd)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }
}

