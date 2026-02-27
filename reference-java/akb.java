/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akb
extends ajc {
    public akb() {
        super(awt.f);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new aox();
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb2.E) {
            boolean bl2;
            boolean bl3 = ahb2.v(n2, n3, n4);
            int n5 = ahb2.e(n2, n3, n4);
            boolean bl4 = bl2 = (n5 & 1) != 0;
            if (bl3 && !bl2) {
                ahb2.a(n2, n3, n4, n5 | 1, 4);
                ahb2.a(n2, n3, n4, this, this.a(ahb2));
            } else if (!bl3 && bl2) {
                ahb2.a(n2, n3, n4, n5 & 0xFFFFFFFE, 4);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null && aor2 instanceof aox) {
            agp agp2 = ((aox)aor2).a();
            agp2.a(ahb2);
            ahb2.f(n2, n3, n4, this);
        }
    }

    @Override
    public int a(ahb ahb2) {
        return 1;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        aox aox2 = (aox)ahb2.o(n2, n3, n4);
        if (aox2 != null) {
            yz2.a(aox2);
        }
        return true;
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 != null && aor2 instanceof aox) {
            return ((aox)aor2).a().g();
        }
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        aox aox2 = (aox)ahb2.o(n2, n3, n4);
        if (add2.u()) {
            aox2.a().b(add2.s());
        }
    }

    @Override
    public int a(Random random) {
        return 0;
    }
}

