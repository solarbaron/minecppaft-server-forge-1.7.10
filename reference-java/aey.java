/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public class aey
implements afg {
    private add a;

    @Override
    public boolean a(aae aae2, ahb ahb2) {
        Object object;
        this.a = null;
        int n2 = 0;
        int n3 = 0;
        int n4 = 0;
        int n5 = 0;
        int n6 = 0;
        int n7 = 0;
        for (int i2 = 0; i2 < aae2.a(); ++i2) {
            object = aae2.a(i2);
            if (object == null) continue;
            if (((add)object).b() == ade.H) {
                ++n3;
                continue;
            }
            if (((add)object).b() == ade.bQ) {
                ++n5;
                continue;
            }
            if (((add)object).b() == ade.aR) {
                ++n4;
                continue;
            }
            if (((add)object).b() == ade.aF) {
                ++n2;
                continue;
            }
            if (((add)object).b() == ade.aO) {
                ++n6;
                continue;
            }
            if (((add)object).b() == ade.i) {
                ++n6;
                continue;
            }
            if (((add)object).b() == ade.bz) {
                ++n7;
                continue;
            }
            if (((add)object).b() == ade.G) {
                ++n7;
                continue;
            }
            if (((add)object).b() == ade.bl) {
                ++n7;
                continue;
            }
            if (((add)object).b() == ade.bL) {
                ++n7;
                continue;
            }
            return false;
        }
        n6 += n4 + n7;
        if (n3 > 3 || n2 > 1) {
            return false;
        }
        if (n3 >= 1 && n2 == 1 && n6 == 0) {
            this.a = new add(ade.bP);
            if (n5 > 0) {
                dh dh2 = new dh();
                object = new dh();
                dq dq2 = new dq();
                for (int i3 = 0; i3 < aae2.a(); ++i3) {
                    add add2 = aae2.a(i3);
                    if (add2 == null || add2.b() != ade.bQ || !add2.p() || !add2.q().b("Explosion", 10)) continue;
                    dq2.a(add2.q().m("Explosion"));
                }
                ((dh)object).a("Explosions", dq2);
                ((dh)object).a("Flight", (byte)n3);
                dh2.a("Fireworks", (dy)object);
                this.a.d(dh2);
            }
            return true;
        }
        if (n3 == 1 && n2 == 0 && n5 == 0 && n4 > 0 && n7 <= 1) {
            this.a = new add(ade.bQ);
            dh dh3 = new dh();
            object = new dh();
            int n8 = 0;
            ArrayList<Integer> arrayList = new ArrayList<Integer>();
            for (int i4 = 0; i4 < aae2.a(); ++i4) {
                add add3 = aae2.a(i4);
                if (add3 == null) continue;
                if (add3.b() == ade.aR) {
                    arrayList.add(acj.c[add3.k()]);
                    continue;
                }
                if (add3.b() == ade.aO) {
                    ((dh)object).a("Flicker", true);
                    continue;
                }
                if (add3.b() == ade.i) {
                    ((dh)object).a("Trail", true);
                    continue;
                }
                if (add3.b() == ade.bz) {
                    n8 = 1;
                    continue;
                }
                if (add3.b() == ade.G) {
                    n8 = 4;
                    continue;
                }
                if (add3.b() == ade.bl) {
                    n8 = 2;
                    continue;
                }
                if (add3.b() != ade.bL) continue;
                n8 = 3;
            }
            int[] nArray = new int[arrayList.size()];
            for (int i5 = 0; i5 < nArray.length; ++i5) {
                nArray[i5] = (Integer)arrayList.get(i5);
            }
            ((dh)object).a("Colors", nArray);
            ((dh)object).a("Type", (byte)n8);
            dh3.a("Explosion", (dy)object);
            this.a.d(dh3);
            return true;
        }
        if (n3 == 0 && n2 == 0 && n5 == 1 && n4 > 0 && n4 == n6) {
            dh dh4;
            ArrayList<Integer> arrayList = new ArrayList<Integer>();
            for (int i6 = 0; i6 < aae2.a(); ++i6) {
                add add4 = aae2.a(i6);
                if (add4 == null) continue;
                if (add4.b() == ade.aR) {
                    arrayList.add(acj.c[add4.k()]);
                    continue;
                }
                if (add4.b() != ade.bQ) continue;
                this.a = add4.m();
                this.a.b = 1;
            }
            int[] nArray = new int[arrayList.size()];
            for (int i7 = 0; i7 < nArray.length; ++i7) {
                nArray[i7] = (Integer)arrayList.get(i7);
            }
            if (this.a != null && this.a.p()) {
                dh4 = this.a.q().m("Explosion");
                if (dh4 == null) {
                    return false;
                }
            } else {
                return false;
            }
            dh4.a("FadeColors", nArray);
            return true;
        }
        return false;
    }

    @Override
    public add a(aae aae2) {
        return this.a.m();
    }

    @Override
    public int a() {
        return 10;
    }

    @Override
    public add b() {
        return this.a;
    }
}

