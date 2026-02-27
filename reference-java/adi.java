/*
 * Decompiled with CFR 0.152.
 */
public class adi
extends adb {
    public adi() {
        this.e(1);
        this.a(abt.f);
    }

    @Override
    public add b(add add2, ahb ahb2, yz yz2) {
        if (!yz2.bE.d) {
            --add2.b;
        }
        if (!ahb2.E) {
            yz2.aP();
        }
        if (add2.b <= 0) {
            return new add(ade.ar);
        }
        return add2;
    }

    @Override
    public int d_(add add2) {
        return 32;
    }

    @Override
    public aei d(add add2) {
        return aei.c;
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        yz2.a(add2, this.d_(add2));
        return add2;
    }
}

