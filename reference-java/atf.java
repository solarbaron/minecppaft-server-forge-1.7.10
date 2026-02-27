/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class atf {
    private static final att[] a = new att[]{new att(ati.class, 30, 0, true), new att(atg.class, 10, 4), new att(atu.class, 10, 4), new att(atv.class, 10, 3), new att(atr.class, 5, 2), new att(atl.class, 5, 1)};
    private static final att[] b = new att[]{new att(ato.class, 25, 0, true), new att(atm.class, 15, 5), new att(atp.class, 5, 10), new att(atn.class, 5, 10), new att(atj.class, 10, 3, true), new att(atk.class, 7, 2), new att(atq.class, 5, 2)};

    public static void a() {
        avi.a(atg.class, "NeBCr");
        avi.a(ath.class, "NeBEF");
        avi.a(ati.class, "NeBS");
        avi.a(atj.class, "NeCCS");
        avi.a(atk.class, "NeCTB");
        avi.a(atl.class, "NeCE");
        avi.a(atm.class, "NeSCSC");
        avi.a(atn.class, "NeSCLT");
        avi.a(ato.class, "NeSC");
        avi.a(atp.class, "NeSCRT");
        avi.a(atq.class, "NeCSR");
        avi.a(atr.class, "NeMT");
        avi.a(atu.class, "NeRC");
        avi.a(atv.class, "NeSR");
        avi.a(atw.class, "NeStart");
    }

    private static ats b(att att2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        Class clazz = att2.a;
        ats ats2 = null;
        if (clazz == ati.class) {
            ats2 = ati.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atg.class) {
            ats2 = atg.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atu.class) {
            ats2 = atu.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atv.class) {
            ats2 = atv.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atr.class) {
            ats2 = atr.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atl.class) {
            ats2 = atl.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == ato.class) {
            ats2 = ato.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atp.class) {
            ats2 = atp.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atn.class) {
            ats2 = atn.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atj.class) {
            ats2 = atj.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atk.class) {
            ats2 = atk.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atm.class) {
            ats2 = atm.a(list, random, n2, n3, n4, n5, n6);
        } else if (clazz == atq.class) {
            ats2 = atq.a(list, random, n2, n3, n4, n5, n6);
        }
        return ats2;
    }

    static /* synthetic */ ats a(att att2, List list, Random random, int n2, int n3, int n4, int n5, int n6) {
        return atf.b(att2, list, random, n2, n3, n4, n5, n6);
    }

    static /* synthetic */ att[] b() {
        return a;
    }

    static /* synthetic */ att[] c() {
        return b;
    }
}

