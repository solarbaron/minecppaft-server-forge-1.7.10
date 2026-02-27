/*
 * Decompiled with CFR 0.152.
 */
public class ajg
extends ajc {
    public ajg() {
        super(awt.s);
        this.c(3.0f);
        this.a(abt.f);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new aoq();
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        aoq aoq2 = (aoq)ahb2.o(n2, n3, n4);
        if (aoq2 != null) {
            yz2.a(aoq2);
        }
        return true;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 34;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        super.a(ahb2, n2, n3, n4, sv2, add2);
        if (add2.u()) {
            ((aoq)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }
}

