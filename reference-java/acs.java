/*
 * Decompiled with CFR 0.152.
 */
public class acs
extends adb {
    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (!ahb2.E) {
            zf zf2 = new zf(ahb2, (float)n2 + f2, (float)n3 + f3, (float)n4 + f4, add2);
            ahb2.d(zf2);
            if (!yz2.bE.d) {
                --add2.b;
            }
            return true;
        }
        return false;
    }
}

