/*
 * Decompiled with CFR 0.152.
 */
public class aaz
extends zs {
    private apb a;

    public aaz(rb rb2, apb apb2) {
        int n2;
        int n3;
        this.a = apb2;
        for (n3 = 0; n3 < 3; ++n3) {
            for (n2 = 0; n2 < 3; ++n2) {
                this.a(new aay(apb2, n2 + n3 * 3, 62 + n2 * 18, 17 + n3 * 18));
            }
        }
        for (n3 = 0; n3 < 3; ++n3) {
            for (n2 = 0; n2 < 9; ++n2) {
                this.a(new aay(rb2, n2 + n3 * 9 + 9, 8 + n2 * 18, 84 + n3 * 18));
            }
        }
        for (n3 = 0; n3 < 9; ++n3) {
            this.a(new aay(rb2, n3, 8 + n3 * 18, 142));
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
            if (n2 < 9 ? !this.a(add3, 9, 45, true) : !this.a(add3, 0, 9, false)) {
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
}

