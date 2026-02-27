/*
 * Decompiled with CFR 0.152.
 */
public class aaf
extends zs {
    public aae a = new aae(this, 3, 3);
    public rb f = new aaw();
    private ahb g;
    private int h;
    private int i;
    private int j;

    public aaf(yx yx2, ahb ahb2, int n2, int n3, int n4) {
        int n5;
        int n6;
        this.g = ahb2;
        this.h = n2;
        this.i = n3;
        this.j = n4;
        this.a(new aax(yx2.d, this.a, this.f, 0, 124, 35));
        for (n6 = 0; n6 < 3; ++n6) {
            for (n5 = 0; n5 < 3; ++n5) {
                this.a(new aay(this.a, n5 + n6 * 3, 30 + n5 * 18, 17 + n6 * 18));
            }
        }
        for (n6 = 0; n6 < 3; ++n6) {
            for (n5 = 0; n5 < 9; ++n5) {
                this.a(new aay(yx2, n5 + n6 * 9 + 9, 8 + n5 * 18, 84 + n6 * 18));
            }
        }
        for (n6 = 0; n6 < 9; ++n6) {
            this.a(new aay(yx2, n6, 8 + n6 * 18, 142));
        }
        this.a(this.a);
    }

    @Override
    public void a(rb rb2) {
        this.f.a(0, afe.a().a(this.a, this.g));
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        if (this.g.E) {
            return;
        }
        for (int i2 = 0; i2 < 9; ++i2) {
            add add2 = this.a.a_(i2);
            if (add2 == null) continue;
            yz2.a(add2, false);
        }
    }

    @Override
    public boolean a(yz yz2) {
        if (this.g.a(this.h, this.i, this.j) != ajn.ai) {
            return false;
        }
        return !(yz2.e((double)this.h + 0.5, (double)this.i + 0.5, (double)this.j + 0.5) > 64.0);
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 == 0) {
                if (!this.a(add3, 10, 46, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (n2 >= 10 && n2 < 37 ? !this.a(add3, 37, 46, false) : (n2 >= 37 && n2 < 46 ? !this.a(add3, 10, 37, false) : !this.a(add3, 10, 46, false))) {
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
    public boolean a(add add2, aay aay2) {
        return aay2.f != this.f && super.a(add2, aay2);
    }
}

