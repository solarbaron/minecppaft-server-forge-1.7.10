/*
 * Decompiled with CFR 0.152.
 */
public class zx
extends zs {
    private aoq a;
    private final zy f;
    private int g;
    private int h;
    private int i;

    public zx(yx yx2, aoq aoq2) {
        int n2;
        this.a = aoq2;
        this.f = new zy(this, aoq2, 0, 136, 110);
        this.a(this.f);
        int n3 = 36;
        int n4 = 137;
        for (n2 = 0; n2 < 3; ++n2) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n2 * 9 + 9, n3 + i2 * 18, n4 + n2 * 18));
            }
        }
        for (n2 = 0; n2 < 9; ++n2) {
            this.a(new aay(yx2, n2, n3 + n2 * 18, 58 + n4));
        }
        this.g = aoq2.l();
        this.h = aoq2.j();
        this.i = aoq2.k();
    }

    @Override
    public void a(aac aac2) {
        super.a(aac2);
        aac2.a((zs)this, 0, this.g);
        aac2.a((zs)this, 1, this.h);
        aac2.a((zs)this, 2, this.i);
    }

    public aoq e() {
        return this.a;
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
            if (n2 == 0) {
                if (!this.a(add3, 1, 37, true)) {
                    return null;
                }
                aay2.a(add3, add2);
            } else if (!this.f.e() && this.f.a(add3) && add3.b == 1 ? !this.a(add3, 0, 1, false) : (n2 >= 1 && n2 < 28 ? !this.a(add3, 28, 37, false) : (n2 >= 28 && n2 < 37 ? !this.a(add3, 1, 28, false) : !this.a(add3, 1, 37, false)))) {
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

