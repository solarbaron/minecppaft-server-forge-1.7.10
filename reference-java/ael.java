/*
 * Decompiled with CFR 0.152.
 */
public class ael
extends adb {
    public ael() {
        this.e(1);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        yz2.b(add2);
        return add2;
    }

    @Override
    public boolean s() {
        return true;
    }

    public static boolean a(dh dh2) {
        if (dh2 == null) {
            return false;
        }
        if (!dh2.b("pages", 9)) {
            return false;
        }
        dq dq2 = dh2.c("pages", 8);
        for (int i2 = 0; i2 < dq2.c(); ++i2) {
            String string = dq2.f(i2);
            if (string == null) {
                return false;
            }
            if (string.length() <= 256) continue;
            return false;
        }
        return true;
    }
}

