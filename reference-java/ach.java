/*
 * Decompiled with CFR 0.152.
 */
public class ach
extends adb {
    private awt a;

    public ach(awt awt2) {
        this.a = awt2;
        this.h = 1;
        this.a(abt.d);
    }

    @Override
    public boolean a(add add2, yz yz2, ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4) {
        if (n5 != 1) {
            return false;
        }
        aji aji2 = this.a == awt.d ? ajn.ao : ajn.av;
        if (!yz2.a(n2, ++n3, n4, n5, add2) || !yz2.a(n2, n3 + 1, n4, n5, add2)) {
            return false;
        }
        if (!aji2.c(ahb2, n2, n3, n4)) {
            return false;
        }
        int n6 = qh.c((double)((yz2.y + 180.0f) * 4.0f / 360.0f) - 0.5) & 3;
        ach.a(ahb2, n2, n3, n4, n6, aji2);
        --add2.b;
        return true;
    }

    public static void a(ahb ahb2, int n2, int n3, int n4, int n5, aji aji2) {
        int n6 = 0;
        int n7 = 0;
        if (n5 == 0) {
            n7 = 1;
        }
        if (n5 == 1) {
            n6 = -1;
        }
        if (n5 == 2) {
            n7 = -1;
        }
        if (n5 == 3) {
            n6 = 1;
        }
        int n8 = (ahb2.a(n2 - n6, n3, n4 - n7).r() ? 1 : 0) + (ahb2.a(n2 - n6, n3 + 1, n4 - n7).r() ? 1 : 0);
        int n9 = (ahb2.a(n2 + n6, n3, n4 + n7).r() ? 1 : 0) + (ahb2.a(n2 + n6, n3 + 1, n4 + n7).r() ? 1 : 0);
        boolean bl2 = ahb2.a(n2 - n6, n3, n4 - n7) == aji2 || ahb2.a(n2 - n6, n3 + 1, n4 - n7) == aji2;
        boolean bl3 = ahb2.a(n2 + n6, n3, n4 + n7) == aji2 || ahb2.a(n2 + n6, n3 + 1, n4 + n7) == aji2;
        boolean bl4 = false;
        if (bl2 && !bl3) {
            bl4 = true;
        } else if (n9 > n8) {
            bl4 = true;
        }
        ahb2.d(n2, n3, n4, aji2, n5, 2);
        ahb2.d(n2, n3 + 1, n4, aji2, 8 | (bl4 ? 1 : 0), 2);
        ahb2.d(n2, n3, n4, aji2);
        ahb2.d(n2, n3 + 1, n4, aji2);
    }
}

