/*
 * Decompiled with CFR 0.152.
 */
public class aat
extends zs {
    private agm a;
    private aas f;
    private final ahb g;

    public aat(yx yx2, agm agm2, ahb ahb2) {
        int n2;
        this.a = agm2;
        this.g = ahb2;
        this.f = new aas(yx2.d, agm2);
        this.a(new aay(this.f, 0, 36, 53));
        this.a(new aay(this.f, 1, 62, 53));
        this.a(new aau(yx2.d, agm2, this.f, 2, 120, 53));
        for (n2 = 0; n2 < 3; ++n2) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n2 * 9 + 9, 8 + i2 * 18, 84 + n2 * 18));
            }
        }
        for (n2 = 0; n2 < 9; ++n2) {
            this.a(new aay(yx2, n2, 8 + n2 * 18, 142));
        }
    }

    public aas e() {
        return this.f;
    }

    @Override
    public void a(aac aac2) {
        super.a(aac2);
    }

    @Override
    public void b() {
        super.b();
    }

    @Override
    public void a(rb rb2) {
        this.f.h();
        super.a(rb2);
    }

    public void e(int n2) {
        this.f.c(n2);
    }

    @Override
    public boolean a(yz yz2) {
        return this.a.b() == yz2;
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 == 2) {
                if (!this.a(add3, 3, 39, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (n2 == 0 || n2 == 1 ? !this.a(add3, 3, 39, false) : (n2 >= 3 && n2 < 30 ? !this.a(add3, 30, 39, false) : n2 >= 30 && n2 < 39 && !this.a(add3, 3, 30, false))) {
                return null;
            }
            if (add3.b == 0) {
                aay2.c(null);
            } else {
                aay2.f();
            }
            if (add3.b == add2.b) {
                return null;
            }
            aay2.a(yz2, add3);
        }
        return add2;
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        this.a.a_((yz)null);
        super.b(yz2);
        if (this.g.E) {
            return;
        }
        add add2 = this.f.a_(0);
        if (add2 != null) {
            yz2.a(add2, false);
        }
        if ((add2 = this.f.a_(1)) != null) {
            yz2.a(add2, false);
        }
    }
}

