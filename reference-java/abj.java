/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class abj
extends adb {
    public abj() {
        this.h = 1;
        this.a(abt.e);
    }

    @Override
    public add a(add add2, ahb ahb2, yz yz2) {
        Object object;
        int n2;
        float f2;
        float f3;
        float f4;
        double d2;
        float f5;
        float f6 = 1.0f;
        float f7 = yz2.B + (yz2.z - yz2.B) * f6;
        float f8 = yz2.A + (yz2.y - yz2.A) * f6;
        double d3 = yz2.p + (yz2.s - yz2.p) * (double)f6;
        double d4 = yz2.q + (yz2.t - yz2.q) * (double)f6 + 1.62 - (double)yz2.L;
        double d5 = yz2.r + (yz2.u - yz2.r) * (double)f6;
        azw azw2 = azw.a(d3, d4, d5);
        float f9 = qh.b(-f8 * ((float)Math.PI / 180) - (float)Math.PI);
        float f10 = qh.a(-f8 * ((float)Math.PI / 180) - (float)Math.PI);
        float f11 = f10 * (f5 = -qh.b(-f7 * ((float)Math.PI / 180)));
        azw azw3 = azw2.c((double)f11 * (d2 = 5.0), (double)(f4 = (f3 = qh.a(-f7 * ((float)Math.PI / 180)))) * d2, (double)(f2 = f9 * f5) * d2);
        azu azu2 = ahb2.a(azw2, azw3, true);
        if (azu2 == null) {
            return add2;
        }
        azw azw4 = yz2.j(f6);
        boolean bl2 = false;
        float f12 = 1.0f;
        List list = ahb2.b((sa)yz2, yz2.C.a(azw4.a * d2, azw4.b * d2, azw4.c * d2).b(f12, f12, f12));
        for (n2 = 0; n2 < list.size(); ++n2) {
            float f13;
            sa sa2 = (sa)list.get(n2);
            if (!sa2.R() || !((azt)(object = sa2.C.b(f13 = sa2.af(), f13, f13))).a(azw2)) continue;
            bl2 = true;
        }
        if (bl2) {
            return add2;
        }
        if (azu2.a == azv.b) {
            n2 = azu2.b;
            int n3 = azu2.c;
            int n4 = azu2.d;
            if (ahb2.a(n2, n3, n4) == ajn.aC) {
                --n3;
            }
            object = new xi(ahb2, (float)n2 + 0.5f, (float)n3 + 1.0f, (float)n4 + 0.5f);
            ((xi)object).y = ((qh.c((double)(yz2.y * 4.0f / 360.0f) + 0.5) & 3) - 1) * 90;
            if (!ahb2.a((sa)object, ((xi)object).C.b(-0.1, -0.1, -0.1)).isEmpty()) {
                return add2;
            }
            if (!ahb2.E) {
                ahb2.d((sa)object);
            }
            if (!yz2.bE.d) {
                --add2.b;
            }
        }
        return add2;
    }
}

