/*
 * Decompiled with CFR 0.152.
 */
public class adm
extends adb {
    public adm() {
        this.a(abt.i);
    }

    @Override
    public boolean a(add add2, yz yz2, sv sv2) {
        if (!add2.u()) {
            return false;
        }
        if (sv2 instanceof sw) {
            sw sw2 = (sw)sv2;
            sw2.a(add2.s());
            sw2.bF();
            --add2.b;
            return true;
        }
        return super.a(add2, yz2, sv2);
    }
}

