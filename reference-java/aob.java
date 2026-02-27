/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class aob
extends aji {
    public aob() {
        super(awt.u);
        this.a(abt.d);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        if (ahb2.v(n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, 1);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (ahb2.v(n2, n3, n4)) {
            this.b(ahb2, n2, n3, n4, 1);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public int a(Random random) {
        return 1;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, agw agw2) {
        if (ahb2.E) {
            return;
        }
        xw xw2 = new xw(ahb2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, agw2.c());
        xw2.a = ahb2.s.nextInt(xw2.a / 4) + xw2.a / 8;
        ahb2.d(xw2);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, int n5) {
        this.a(ahb2, n2, n3, n4, n5, (sv)null);
    }

    public void a(ahb ahb2, int n2, int n3, int n4, int n5, sv sv2) {
        if (ahb2.E) {
            return;
        }
        if ((n5 & 1) == 1) {
            xw xw2 = new xw(ahb2, (float)n2 + 0.5f, (float)n3 + 0.5f, (float)n4 + 0.5f, sv2);
            ahb2.d(xw2);
            ahb2.a(xw2, "game.tnt.primed", 1.0f, 1.0f);
        }
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (yz2.bF() != null && yz2.bF().b() == ade.d) {
            this.a(ahb2, n2, n3, n4, 1, (sv)yz2);
            ahb2.f(n2, n3, n4);
            yz2.bF().a(1, (sv)yz2);
            return true;
        }
        return super.a(ahb2, n2, n3, n4, yz2, n5, f2, f3, f4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        zc zc2;
        if (sa2 instanceof zc && !ahb2.E && (zc2 = (zc)sa2).al()) {
            this.a(ahb2, n2, n3, n4, 1, zc2.c instanceof sv ? (sv)zc2.c : null);
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public boolean a(agw agw2) {
        return false;
    }
}

