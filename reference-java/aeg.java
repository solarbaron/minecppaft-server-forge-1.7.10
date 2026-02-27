/*
 * Decompiled with CFR 0.152.
 */
public class aeg
extends abh {
    private final boolean b;
    private final alj c;
    private final alj d;

    public aeg(aji aji2, alj alj2, alj alj3, boolean bl2) {
        super(aji2);
        this.c = alj2;
        this.d = alj3;
        this.b = bl2;
        this.f(0);
        this.a(true);
    }

    @Override
    public int a(int n2) {
        return n2;
    }

    @Override
    public String a(add add2) {
        return this.c.b(add2.k());
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        boolean bl2;
        if (this.b) {
            return super.a(add2, yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
        }
        if (add2.b == 0) {
            return false;
        }
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = n6 & 7;
        boolean bl3 = bl2 = (n6 & 8) != 0;
        if ((n5 == 1 && !bl2 || n5 == 0 && bl2) && aji2 == this.c && n7 == add2.k()) {
            if (ahb2.b(this.d.a(ahb2, n2, n3, n4)) && ahb2.d(n2, n3, n4, this.d, n7, 3)) {
                ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), this.d.H.b(), (this.d.H.c() + 1.0f) / 2.0f, this.d.H.d() * 0.8f);
                --add2.b;
            }
            return true;
        }
        if (this.a(add2, yz2, ahb2, n2, n3, n4, n5)) {
            return true;
        }
        return super.a(add2, yz2, ahb2, n2, n3, n4, n5, f2, f3, f4);
    }

    private boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5) {
        if (n5 == 0) {
            --n3;
        }
        if (n5 == 1) {
            ++n3;
        }
        if (n5 == 2) {
            --n4;
        }
        if (n5 == 3) {
            ++n4;
        }
        if (n5 == 4) {
            --n2;
        }
        if (n5 == 5) {
            ++n2;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        int n6 = ahb2.e(n2, n3, n4);
        int n7 = n6 & 7;
        if (aji2 == this.c && n7 == add2.k()) {
            if (ahb2.b(this.d.a(ahb2, n2, n3, n4)) && ahb2.d(n2, n3, n4, this.d, n7, 3)) {
                ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), this.d.H.b(), (this.d.H.c() + 1.0f) / 2.0f, this.d.H.d() * 0.8f);
                --add2.b;
            }
            return true;
        }
        return false;
    }
}

