/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;

public class aeu
implements afg {
    @Override
    public boolean a(aae aae2, ahb ahb2) {
        add add2 = null;
        ArrayList<add> arrayList = new ArrayList<add>();
        for (int i2 = 0; i2 < aae2.a(); ++i2) {
            add add3 = aae2.a(i2);
            if (add3 == null) continue;
            if (add3.b() instanceof abb) {
                abb abb2 = (abb)add3.b();
                if (abb2.m_() == abd.a && add2 == null) {
                    add2 = add3;
                    continue;
                }
                return false;
            }
            if (add3.b() == ade.aR) {
                arrayList.add(add3);
                continue;
            }
            return false;
        }
        return add2 != null && !arrayList.isEmpty();
    }

    @Override
    public add a(aae aae2) {
        float f2;
        float f3;
        int n2;
        int n3;
        add add2 = null;
        int[] nArray = new int[3];
        int n4 = 0;
        int n5 = 0;
        abb abb2 = null;
        for (n3 = 0; n3 < aae2.a(); ++n3) {
            add add3 = aae2.a(n3);
            if (add3 == null) continue;
            if (add3.b() instanceof abb) {
                abb2 = (abb)add3.b();
                if (abb2.m_() == abd.a && add2 == null) {
                    add2 = add3.m();
                    add2.b = 1;
                    if (!abb2.c_(add3)) continue;
                    n2 = abb2.b(add2);
                    f3 = (float)(n2 >> 16 & 0xFF) / 255.0f;
                    f2 = (float)(n2 >> 8 & 0xFF) / 255.0f;
                    float f4 = (float)(n2 & 0xFF) / 255.0f;
                    n4 = (int)((float)n4 + Math.max(f3, Math.max(f2, f4)) * 255.0f);
                    nArray[0] = (int)((float)nArray[0] + f3 * 255.0f);
                    nArray[1] = (int)((float)nArray[1] + f2 * 255.0f);
                    nArray[2] = (int)((float)nArray[2] + f4 * 255.0f);
                    ++n5;
                    continue;
                }
                return null;
            }
            if (add3.b() == ade.aR) {
                float[] fArray = wp.bp[aka.b(add3.k())];
                int n6 = (int)(fArray[0] * 255.0f);
                int n7 = (int)(fArray[1] * 255.0f);
                int n8 = (int)(fArray[2] * 255.0f);
                n4 += Math.max(n6, Math.max(n7, n8));
                nArray[0] = nArray[0] + n6;
                nArray[1] = nArray[1] + n7;
                nArray[2] = nArray[2] + n8;
                ++n5;
                continue;
            }
            return null;
        }
        if (abb2 == null) {
            return null;
        }
        n3 = nArray[0] / n5;
        int n9 = nArray[1] / n5;
        n2 = nArray[2] / n5;
        f3 = (float)n4 / (float)n5;
        f2 = Math.max(n3, Math.max(n9, n2));
        n3 = (int)((float)n3 * f3 / f2);
        n9 = (int)((float)n9 * f3 / f2);
        n2 = (int)((float)n2 * f3 / f2);
        int n10 = n3;
        n10 = (n10 << 8) + n9;
        n10 = (n10 << 8) + n2;
        abb2.b(add2, n10);
        return add2;
    }

    @Override
    public int a() {
        return 10;
    }

    @Override
    public add b() {
        return null;
    }
}

