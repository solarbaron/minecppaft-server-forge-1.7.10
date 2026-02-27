/*
 * Decompiled with CFR 0.152.
 */
public class acv
extends adb {
    public acv() {
        this.f(64);
        this.e(1);
        this.a(abt.i);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (yz2.bK != null) {
            int n2 = yz2.bK.e();
            add2.a(n2, (sv)yz2);
            yz2.ba();
        } else {
            ahb2.a((sa)yz2, "random.bow", 0.5f, 0.4f / (g.nextFloat() * 0.4f + 0.8f));
            if (!ahb2.E) {
                ahb2.d(new xe(ahb2, yz2));
            }
            yz2.ba();
        }
        return add2;
    }

    @Override
    public boolean e_(add add2) {
        return super.e_(add2);
    }

    @Override
    public int c() {
        return 1;
    }
}

