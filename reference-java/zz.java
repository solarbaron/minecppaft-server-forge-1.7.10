/*
 * Decompiled with CFR 0.152.
 */
public class zz
extends zs {
    private aov a;
    private final aay f;
    private int g;

    public zz(yx yx2, aov aov2) {
        int n2;
        this.a = aov2;
        this.a(new aab(yx2.d, aov2, 0, 56, 46));
        this.a(new aab(yx2.d, aov2, 1, 79, 53));
        this.a(new aab(yx2.d, aov2, 2, 102, 46));
        this.f = this.a(new aaa(this, aov2, 3, 79, 17));
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
        aac2.a((zs)this, 0, this.a.i());
    }

    @Override
    public void b() {
        super.b();
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            aac aac2 = (aac)this.e.get(i2);
            if (this.g == this.a.i()) continue;
            aac2.a((zs)this, 0, this.a.i());
        }
        this.g = this.a.i();
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
            if (n2 >= 0 && n2 <= 2 || n2 == 3) {
                if (!this.a(add3, 4, 40, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (!this.f.e() && this.f.a(add3) ? !this.a(add3, 3, 4, false) : (aab.b_(add2) ? !this.a(add3, 0, 3, false) : (n2 >= 4 && n2 < 31 ? !this.a(add3, 31, 40, false) : (n2 >= 31 && n2 < 40 ? !this.a(add3, 4, 31, false) : !this.a(add3, 4, 40, false))))) {
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

