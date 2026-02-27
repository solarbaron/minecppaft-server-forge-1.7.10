/*
 * Decompiled with CFR 0.152.
 */
class aab
extends aay {
    private yz a;

    public aab(yz yz2, rb rb2, int n2, int n3, int n4) {
        super(rb2, n2, n3, n4);
        this.a = yz2;
    }

    @Override
    public boolean a(add add2) {
        return aab.b_(add2);
    }

    @Override
    public int a() {
        return 1;
    }

    @Override
    public void a(yz yz2, add add2) {
        if (add2.b() == ade.bn && add2.k() > 0) {
            this.a.a(pc.B, 1);
        }
        super.a(yz2, add2);
    }

    public static boolean b_(add add2) {
        return add2 != null && (add2.b() == ade.bn || add2.b() == ade.bo);
    }
}

