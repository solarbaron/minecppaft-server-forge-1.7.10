/*
 * Decompiled with CFR 0.152.
 */
public class aco
extends adb {
    public aco() {
        this.h = 16;
        this.a(abt.f);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        if (yz2.bE.d) {
            return add2;
        }
        --add2.b;
        ahb2.a((sa)yz2, "random.bow", 0.5f, 0.4f / (g.nextFloat() * 0.4f + 0.8f));
        if (!ahb2.E) {
            ahb2.d(new zm(ahb2, yz2));
        }
        return add2;
    }
}

