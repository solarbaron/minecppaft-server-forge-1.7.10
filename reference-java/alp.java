/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class alp
extends alk {
    public alp() {
        super("ice", awt.w, false);
        this.K = 0.98f;
        this.a(true);
        this.a(abt.b);
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        yz2.a(pp.C[aji.b(this)], 1);
        yz2.a(0.025f);
        if (this.E() && afv.e(yz2)) {
            add add2 = this.j(n5);
            if (add2 != null) {
                this.a(ahb2, n2, n3, n4, add2);
            }
        } else {
            if (ahb2.t.f) {
                ahb2.f(n2, n3, n4);
                return;
            }
            int n6 = afv.f(yz2);
            this.b(ahb2, n2, n3, n4, n5, n6);
            awt awt2 = ahb2.a(n2, n3 - 1, n4).o();
            if (awt2.c() || awt2.d()) {
                ahb2.b(n2, n3, n4, ajn.i);
            }
        }
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        if (ahb2.b(ahn.b, n2, n3, n4) > 11 - this.k()) {
            if (ahb2.t.f) {
                ahb2.f(n2, n3, n4);
                return;
            }
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.b(n2, n3, n4, ajn.j);
        }
    }

    @Override
    public int h() {
        return 0;
    }
}

