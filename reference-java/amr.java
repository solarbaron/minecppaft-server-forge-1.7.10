/*
 * Decompiled with CFR 0.152.
 */
public class amr
extends akf {
    @Override
    protected adb i() {
        return ade.bG;
    }

    @Override
    protected adb P() {
        return ade.bG;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        super.a(ahb2, n2, n3, n4, n5, f2, n6);
        if (ahb2.E) {
            return;
        }
        if (n5 >= 7 && ahb2.s.nextInt(50) == 0) {
            this.a(ahb2, n2, n3, n4, new add(ade.bI));
        }
    }
}

