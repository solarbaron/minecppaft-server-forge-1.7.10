/*
 * Decompiled with CFR 0.152.
 */
public class aed
extends adb {
    public aed() {
        this.h = 16;
        this.a(abt.f);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (!yz2.bE.d) {
            --add2.b;
        }
        ahb2.a((sa)yz2, "random.bow", 0.5f, 0.4f / (g.nextFloat() * 0.4f + 0.8f));
        if (!ahb2.E) {
            ahb2.d(new zj(ahb2, yz2));
        }
        return add2;
    }
}

