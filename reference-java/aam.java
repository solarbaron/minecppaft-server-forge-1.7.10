/*
 * Decompiled with CFR 0.152.
 */
public class aam
extends zs {
    private rb a;
    private wi f;

    public aam(rb rb2, rb rb3, wi wi2) {
        int n2;
        int n3;
        this.a = rb3;
        this.f = wi2;
        int n4 = 3;
        rb3.f();
        int n5 = (n4 - 4) * 18;
        this.a(new aan(this, rb3, 0, 8, 18));
        this.a(new aao(this, rb3, 1, 8, 36, wi2));
        if (wi2.ck()) {
            for (n3 = 0; n3 < n4; ++n3) {
                for (n2 = 0; n2 < 5; ++n2) {
                    this.a(new aay(rb3, 2 + n2 + n3 * 5, 80 + n2 * 18, 18 + n3 * 18));
                }
            }
        }
        for (n3 = 0; n3 < 3; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.a(new aay(rb2, n2 + n3 * 9 + 9, 8 + n2 * 18, 102 + n3 * 18 + n5));
            }
        }
        for (n3 = 0; n3 < 9; ++n3) {
            this.a(new aay(rb2, n3, 8 + n3 * 18, 160 + n5));
        }
    }

    @Override
    public boolean a(yz yz2) {
        return this.a.a(yz2) && this.f.Z() && this.f.e((sa)yz2) < 8.0f;
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 < this.a.a() ? !this.a(add3, this.a.a(), this.c.size(), true) : (this.a(1).a(add3) && !this.a(1).e() ? !this.a(add3, 1, 2, false) : (this.a(0).a(add3) ? !this.a(add3, 0, 1, false) : this.a.a() <= 2 || !this.a(add3, 2, this.a.a(), false)))) {
                return null;
            }
            if (add3.b == 0) {
                aay2.c(null);
            } else {
                aay2.f();
            }
        }
        return add2;
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        this.a.l_();
    }
}

