/*
 * Decompiled with CFR 0.152.
 */
public class adx
extends adb {
    public adx() {
        this.e(1);
        this.f(238);
        this.a(abt.i);
    }

    @Override
    public boolean a(add add2, ahb ahb2, aji aji2, int n2, int n3, int n4, sv sv2) {
        if (aji2.o() == awt.j || aji2 == ajn.G || aji2 == ajn.H || aji2 == ajn.bd || aji2 == ajn.bD) {
            add2.a(1, sv2);
            return true;
        }
        return super.a(add2, ahb2, aji2, n2, n3, n4, sv2);
    }

    @Override
    public boolean b(aji aji2) {
        return aji2 == ajn.G || aji2 == ajn.af || aji2 == ajn.bD;
    }

    @Override
    public float a(add add2, aji aji2) {
        if (aji2 == ajn.G || aji2.o() == awt.j) {
            return 15.0f;
        }
        if (aji2 == ajn.L) {
            return 5.0f;
        }
        return super.a(add2, aji2);
    }
}

