/*
 * Decompiled with CFR 0.152.
 */
public class ada
extends adb {
    protected adc a;

    public ada(adc adc2) {
        this.a = adc2;
        this.h = 1;
        this.f(adc2.a());
        this.a(abt.i);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        aji aji2 = ahb2.a(n2, n3, n4);
        if (n5 != 0 && ahb2.a(n2, n3 + 1, n4).o() == awt.a && (aji2 == ajn.c || aji2 == ajn.d)) {
            aji aji3 = ajn.ak;
            ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), aji3.H.e(), (aji3.H.c() + 1.0f) / 2.0f, aji3.H.d() * 0.8f);
            if (ahb2.E) {
                return true;
            }
            ahb2.b(n2, n3, n4, aji3);
            add2.a(1, (sv)yz2);
            return true;
        }
        return false;
    }

    public String i() {
        return this.a.toString();
    }
}

