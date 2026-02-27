/*
 * Decompiled with CFR 0.152.
 */
public class adt
extends adb {
    public adt() {
        this.h = 1;
        this.a(abt.e);
    }

    @Override
    public boolean a(add add2, yz yz2, sv sv2) {
        if (sv2 instanceof wo) {
            wo wo2 = (wo)sv2;
            if (!wo2.bZ() && !wo2.f()) {
                wo2.i(true);
                wo2.o.a(wo2, "mob.horse.leather", 0.5f, 1.0f);
                --add2.b;
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean a(add add2, sv sv2, sv sv3) {
        this.a(add2, null, sv2);
        return true;
    }
}

