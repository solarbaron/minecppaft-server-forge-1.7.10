/*
 * Decompiled with CFR 0.152.
 */
public class acj
extends adb {
    public static final String[] a = new String[]{"black", "red", "green", "brown", "blue", "purple", "cyan", "silver", "gray", "pink", "lime", "yellow", "lightBlue", "magenta", "orange", "white"};
    public static final String[] b = new String[]{"black", "red", "green", "brown", "blue", "purple", "cyan", "silver", "gray", "pink", "lime", "yellow", "light_blue", "magenta", "orange", "white"};
    public static final int[] c = new int[]{0x1E1B1B, 11743532, 3887386, 5320730, 2437522, 8073150, 2651799, 0xABABAB, 0x434343, 14188952, 4312372, 14602026, 6719955, 12801229, 15435844, 0xF0F0F0};

    public acj() {
        this.a(true);
        this.f(0);
        this.a(abt.l);
    }

    @Override
    public String a(add add2) {
        int n2 = qh.a(add2.k(), 0, 15);
        return super.a() + "." + a[n2];
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (!yz2.a(n2, n3, n4, n5, add2)) {
            return false;
        }
        if (add2.k() == 15) {
            if (acj.a(add2, ahb2, n2, n3, n4)) {
                if (!ahb2.E) {
                    ahb2.c(2005, n2, n3, n4, 0);
                }
                return true;
            }
        } else if (add2.k() == 3) {
            aji aji2 = ahb2.a(n2, n3, n4);
            int n6 = ahb2.e(n2, n3, n4);
            if (aji2 == ajn.r && alx.c(n6) == 3) {
                if (n5 == 0) {
                    return false;
                }
                if (n5 == 1) {
                    return false;
                }
                if (n5 == 2) {
                    --n4;
                }
                if (n5 == 3) {
                    ++n4;
                }
                if (n5 == 4) {
                    --n2;
                }
                if (n5 == 5) {
                    ++n2;
                }
                if (ahb2.c(n2, n3, n4)) {
                    int n7 = ajn.by.a(ahb2, n2, n3, n4, n5, f2, f3, f4, 0);
                    ahb2.d(n2, n3, n4, ajn.by, n7, 2);
                    if (!yz2.bE.d) {
                        --add2.b;
                    }
                }
                return true;
            }
        }
        return false;
    }

    public static boolean a(add add2, ahb ahb2, int n2, int n3, int n4) {
        ajo ajo2;
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2 instanceof ajo && (ajo2 = (ajo)((Object)aji2)).a(ahb2, n2, n3, n4, ahb2.E)) {
            if (!ahb2.E) {
                if (ajo2.a(ahb2, ahb2.s, n2, n3, n4)) {
                    ajo2.b(ahb2, ahb2.s, n2, n3, n4);
                }
                --add2.b;
            }
            return true;
        }
        return false;
    }

    @Override
    public boolean a(add add2, yz yz2, sv sv2) {
        if (sv2 instanceof wp) {
            wp wp2 = (wp)sv2;
            int n2 = aka.b(add2.k());
            if (!wp2.ca() && wp2.bZ() != n2) {
                wp2.s(n2);
                --add2.b;
            }
            return true;
        }
        return false;
    }
}

