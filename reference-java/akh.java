/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akh
extends ajr {
    protected akh() {
        super(awt.l);
        float f2 = 0.4f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, 0.8f, 0.5f + f2);
    }

    @Override
    protected boolean a(aji aji2) {
        return aji2 == ajn.m || aji2 == ajn.ch || aji2 == ajn.ce || aji2 == ajn.d;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E && yz2.bF() != null && yz2.bF().b() == ade.aZ) {
            yz2.a(pp.C[aji.b(this)], 1);
            this.a(ahb2, n2, n3, n4, new add(ajn.I, 1, n5));
        } else {
            super.a(ahb2, yz2, n2, n3, n4, n5);
        }
    }
}

