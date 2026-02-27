/*
 * Decompiled with CFR 0.152.
 */
public class aap
extends zs {
    public aae a = new aae(this, 2, 2);
    public rb f = new aaw();
    public boolean g;
    private final yz h;

    public aap(yx yx2, boolean bl2, yz yz2) {
        int n2;
        int n3;
        this.g = bl2;
        this.h = yz2;
        this.a(new aax(yx2.d, this.a, this.f, 0, 144, 36));
        for (n3 = 0; n3 < 2; ++n3) {
            for (n2 = 0; n2 < 2; ++n2) {
                this.a(new aay(this.a, n2 + n3 * 2, 88 + n2 * 18, 26 + n3 * 18));
            }
        }
        for (n3 = 0; n3 < 4; ++n3) {
            n2 = n3;
            this.a(new aaq(this, yx2, yx2.a() - 1 - n3, 8, 8 + n3 * 18, n2));
        }
        for (n3 = 0; n3 < 3; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.a(new aay(yx2, n2 + (n3 + 1) * 9, 8 + n2 * 18, 84 + n3 * 18));
            }
        }
        for (n3 = 0; n3 < 9; ++n3) {
            this.a(new aay(yx2, n3, 8 + n3 * 18, 142));
        }
        this.a(this.a);
    }

    @Override
    public void a(rb rb2) {
        this.f.a(0, afe.a().a(this.a, this.h.o));
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        for (int i2 = 0; i2 < 4; ++i2) {
            add add2 = this.a.a_(i2);
            if (add2 == null) continue;
            yz2.a(add2, false);
        }
        this.f.a(0, null);
    }

    @Override
    public boolean a(yz yz2) {
        return true;
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            int n3;
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 == 0) {
                if (!this.a(add3, 9, 45, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (n2 >= 1 && n2 < 5 ? !this.a(add3, 9, 45, false) : (n2 >= 5 && n2 < 9 ? !this.a(add3, 9, 45, false) : (add2.b() instanceof abb && !((aay)this.c.get(5 + ((abb)add2.b()).b)).e() ? !this.a(add3, n3 = 5 + ((abb)add2.b()).b, n3 + 1, false) : (n2 >= 9 && n2 < 36 ? !this.a(add3, 36, 45, false) : (n2 >= 36 && n2 < 45 ? !this.a(add3, 9, 36, false) : !this.a(add3, 9, 45, false)))))) {
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

