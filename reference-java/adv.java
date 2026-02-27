/*
 * Decompiled with CFR 0.152.
 */
public class adv
extends acx {
    private aji b;
    private aji c;

    public adv(int n2, float f2, aji aji2, aji aji3) {
        super(n2, f2, false);
        this.b = aji2;
        this.c = aji3;
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 != 1) {
            return false;
        }
        if (!yz2.a(n2, n3, n4, n5, add2) || !yz2.a(n2, n3 + 1, n4, n5, add2)) {
            return false;
        }
        if (ahb2.a(n2, n3, n4) == this.c && ahb2.c(n2, n3 + 1, n4)) {
            ahb2.b(n2, n3 + 1, n4, this.b);
            --add2.b;
            return true;
        }
        return false;
    }
}

