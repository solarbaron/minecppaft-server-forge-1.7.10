/*
 * Decompiled with CFR 0.152.
 */
class aaq
extends aay {
    final /* synthetic */ int a;
    final /* synthetic */ aap b;

    aaq(aap aap2, rb rb2, int n2, int n3, int n4, int n5) {
        this.b = aap2;
        this.a = n5;
        super(rb2, n2, n3, n4);
    }

    @Override
    public int a() {
        return 1;
    }

    @Override
    public boolean a(add add2) {
        if (add2 == null) {
            return false;
        }
        if (add2.b() instanceof abb) {
            return ((abb)add2.b()).b == this.a;
        }
        if (add2.b() == adb.a(ajn.aK) || add2.b() == ade.bL) {
            return this.a == 0;
        }
        return false;
    }
}

