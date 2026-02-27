/*
 * Decompiled with CFR 0.152.
 */
public enum afu {
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k;


    public boolean a(adb adb2) {
        if (this == a) {
            return true;
        }
        if (this == j && adb2.p()) {
            return true;
        }
        if (adb2 instanceof abb) {
            if (this == b) {
                return true;
            }
            abb abb2 = (abb)adb2;
            if (abb2.b == 0) {
                return this == f;
            }
            if (abb2.b == 2) {
                return this == d;
            }
            if (abb2.b == 1) {
                return this == e;
            }
            if (abb2.b == 3) {
                return this == c;
            }
            return false;
        }
        if (adb2 instanceof aeh) {
            return this == g;
        }
        if (adb2 instanceof acg) {
            return this == h;
        }
        if (adb2 instanceof abm) {
            return this == k;
        }
        if (adb2 instanceof acv) {
            return this == i;
        }
        return false;
    }
}

