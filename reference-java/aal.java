/*
 * Decompiled with CFR 0.152.
 */
public class aal
extends zs {
    private final rb a;

    public aal(yx yx2, rb rb2) {
        int n2;
        this.a = rb2;
        rb2.f();
        int n3 = 51;
        for (n2 = 0; n2 < rb2.a(); ++n2) {
            this.a(new aay(rb2, n2, 44 + n2 * 18, 20));
        }
        for (n2 = 0; n2 < 3; ++n2) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n2 * 9 + 9, 8 + i2 * 18, n2 * 18 + n3));
            }
        }
        for (n2 = 0; n2 < 9; ++n2) {
            this.a(new aay(yx2, n2, 8 + n2 * 18, 58 + n3));
        }
    }

    @Override
    public boolean a(yz yz2) {
        return this.a.a(yz2);
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 < this.a.a() ? !this.a(add3, this.a.a(), this.c.size(), true) : !this.a(add3, 0, this.a.a(), false)) {
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

