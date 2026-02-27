/*
 * Decompiled with CFR 0.152.
 */
public class afc
extends afh {
    public afc() {
        super(3, 3, new add[]{new add(ade.aF), new add(ade.aF), new add(ade.aF), new add(ade.aF), new add(ade.aY, 0, Short.MAX_VALUE), new add(ade.aF), new add(ade.aF), new add(ade.aF), new add(ade.aF)}, new add(ade.bJ, 0, 0));
    }

    @Override
    public boolean a(aae aae2, ahb ahb2) {
        if (!super.a(aae2, ahb2)) {
            return false;
        }
        add add2 = null;
        for (int i2 = 0; i2 < aae2.a() && add2 == null; ++i2) {
            add add3 = aae2.a(i2);
            if (add3 == null || add3.b() != ade.aY) continue;
            add2 = add3;
        }
        if (add2 == null) {
            return false;
        }
        ayi ayi2 = ade.aY.a(add2, ahb2);
        if (ayi2 == null) {
            return false;
        }
        return ayi2.d < 4;
    }

    @Override
    public add a(aae aae2) {
        add add2 = null;
        for (int i2 = 0; i2 < aae2.a() && add2 == null; ++i2) {
            add add3 = aae2.a(i2);
            if (add3 == null || add3.b() != ade.aY) continue;
            add2 = add3;
        }
        add2 = add2.m();
        add2.b = 1;
        if (add2.q() == null) {
            add2.d(new dh());
        }
        add2.q().a("map_is_scaling", true);
        return add2;
    }
}

