/*
 * Decompiled with CFR 0.152.
 */
public class aaj
extends zs {
    private apg a;
    private int f;
    private int g;
    private int h;

    public aaj(yx yx2, apg apg2) {
        int n2;
        this.a = apg2;
        this.a(new aay(apg2, 0, 56, 17));
        this.a(new aay(apg2, 1, 56, 53));
        this.a(new aak(yx2.d, apg2, 2, 116, 35));
        for (n2 = 0; n2 < 3; ++n2) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n2 * 9 + 9, 8 + i2 * 18, 84 + n2 * 18));
            }
        }
        for (n2 = 0; n2 < 9; ++n2) {
            this.a(new aay(yx2, n2, 8 + n2 * 18, 142));
        }
    }

    @Override
    public void a(aac aac2) {
        super.a(aac2);
        aac2.a((zs)this, 0, this.a.j);
        aac2.a((zs)this, 1, this.a.a);
        aac2.a((zs)this, 2, this.a.i);
    }

    @Override
    public void b() {
        super.b();
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            aac aac2 = (aac)this.e.get(i2);
            if (this.f != this.a.j) {
                aac2.a((zs)this, 0, this.a.j);
            }
            if (this.g != this.a.a) {
                aac2.a((zs)this, 1, this.a.a);
            }
            if (this.h == this.a.i) continue;
            aac2.a((zs)this, 2, this.a.i);
        }
        this.f = this.a.j;
        this.g = this.a.a;
        this.h = this.a.i;
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
            if (n2 == 2) {
                if (!this.a(add3, 3, 39, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (n2 == 1 || n2 == 0 ? !this.a(add3, 3, 39, false) : (afa.a().a(add3) != null ? !this.a(add3, 0, 1, false) : (apg.b(add3) ? !this.a(add3, 1, 2, false) : (n2 >= 3 && n2 < 30 ? !this.a(add3, 30, 39, false) : n2 >= 30 && n2 < 39 && !this.a(add3, 3, 30, false))))) {
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

