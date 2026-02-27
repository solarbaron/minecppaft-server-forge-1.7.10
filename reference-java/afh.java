/*
 * Decompiled with CFR 0.152.
 */
public class afh
implements afg {
    private int a;
    private int b;
    private add[] c;
    private add d;
    private boolean e;

    public afh(int n2, int n3, add[] addArray, add add2) {
        this.a = n2;
        this.b = n3;
        this.c = addArray;
        this.d = add2;
    }

    @Override
    public add b() {
        return this.d;
    }

    @Override
    public boolean a(aae aae2, ahb ahb2) {
        for (int i2 = 0; i2 <= 3 - this.a; ++i2) {
            for (int i3 = 0; i3 <= 3 - this.b; ++i3) {
                if (this.a(aae2, i2, i3, true)) {
                    return true;
                }
                if (!this.a(aae2, i2, i3, false)) continue;
                return true;
            }
        }
        return false;
    }

    private boolean a(aae aae2, int n2, int n3, boolean bl2) {
        for (int i2 = 0; i2 < 3; ++i2) {
            for (int i3 = 0; i3 < 3; ++i3) {
                add add2;
                int n4 = i2 - n2;
                int n5 = i3 - n3;
                add add3 = null;
                if (n4 >= 0 && n5 >= 0 && n4 < this.a && n5 < this.b) {
                    add3 = bl2 ? this.c[this.a - n4 - 1 + n5 * this.a] : this.c[n4 + n5 * this.a];
                }
                if ((add2 = aae2.b(i2, i3)) == null && add3 == null) continue;
                if (add2 == null && add3 != null || add2 != null && add3 == null) {
                    return false;
                }
                if (add3.b() != add2.b()) {
                    return false;
                }
                if (add3.k() == Short.MAX_VALUE || add3.k() == add2.k()) continue;
                return false;
            }
        }
        return true;
    }

    @Override
    public add a(aae aae2) {
        add add2 = this.b().m();
        if (this.e) {
            for (int i2 = 0; i2 < aae2.a(); ++i2) {
                add add3 = aae2.a(i2);
                if (add3 == null || !add3.p()) continue;
                add2.d((dh)add3.d.b());
            }
        }
        return add2;
    }

    @Override
    public int a() {
        return this.a * this.b;
    }

    public afh c() {
        this.e = true;
        return this;
    }
}

