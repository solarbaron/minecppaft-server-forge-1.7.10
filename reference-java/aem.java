/*
 * Decompiled with CFR 0.152.
 */
public class aem
extends adb {
    public aem() {
        this.e(1);
    }

    public static boolean a(dh dh2) {
        if (!ael.a(dh2)) {
            return false;
        }
        if (!dh2.b("title", 8)) {
            return false;
        }
        String string = dh2.j("title");
        if (string == null || string.length() > 16) {
            return false;
        }
        return dh2.b("author", 8);
    }

    @Override
    public String n(add add2) {
        dh dh2;
        String string;
        if (add2.p() && !qn.b(string = (dh2 = add2.q()).j("title"))) {
            return string;
        }
        return super.n(add2);
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
}

