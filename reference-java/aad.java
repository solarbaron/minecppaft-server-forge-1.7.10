/*
 * Decompiled with CFR 0.152.
 */
public class aad
extends zs {
    private rb a;
    private int f;

    public aad(rb rb2, rb rb3) {
        int n2;
        int n3;
        this.a = rb3;
        this.f = rb3.a() / 9;
        rb3.f();
        int n4 = (this.f - 4) * 18;
        for (n3 = 0; n3 < this.f; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.a(new aay(rb3, n2 + n3 * 9, 8 + n2 * 18, 18 + n3 * 18));
            }
        }
        for (n3 = 0; n3 < 3; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.a(new aay(rb2, n2 + n3 * 9 + 9, 8 + n2 * 18, 103 + n3 * 18 + n4));
            }
        }
        for (n3 = 0; n3 < 9; ++n3) {
            this.a(new aay(rb2, n3, 8 + n3 * 18, 161 + n4));
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
            if (n2 < this.f * 9 ? !this.a(add3, this.f * 9, this.c.size(), true) : !this.a(add3, 0, this.f * 9, false)) {
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

    public rb e() {
        return this.a;
    }
}

