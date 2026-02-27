/*
 * Decompiled with CFR 0.152.
 */
public class ala
extends akk {
    public ala() {
        super(awt.d);
        this.a(abt.d);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb2.a(n2, n3 - 1, n4).o().a()) {
            return false;
        }
        return super.c(ahb2, n2, n3, n4);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        if (ala.b(n5)) {
            return null;
        }
        if (n5 == 2 || n5 == 0) {
            return azt.a(n2, n3, (float)n4 + 0.375f, n2 + 1, (float)n3 + 1.5f, (float)n4 + 0.625f);
        }
        return azt.a((float)n2 + 0.375f, n3, n4, (float)n2 + 0.625f, (float)n3 + 1.5f, n4 + 1);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ala.l(ahl2.e(n2, n3, n4));
        if (n5 == 2 || n5 == 0) {
            this.a(0.0f, 0.0f, 0.375f, 1.0f, 1.0f, 0.625f);
        } else {
            this.a(0.375f, 0.0f, 0.0f, 0.625f, 1.0f, 1.0f);
        }
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        return ala.b(ahl2.e(n2, n3, n4));
    }

    @Override
    public int b() {
        return 21;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = (qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) % 4;
        ahb2.a(n2, n3, n4, n5, 2);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        int n6 = ahb2.e(n2, n3, n4);
        if (ala.b(n6)) {
            ahb2.a(n2, n3, n4, n6 & 0xFFFFFFFB, 2);
        } else {
            int n7 = (qh.c((double)(yz2.y * 4.0f / 360.0f) + 0.5) & 3) % 4;
            int n8 = ala.l(n6);
            if (n8 == (n7 + 2) % 4) {
                n6 = n7;
            }
            ahb2.a(n2, n3, n4, n6 | 4, 2);
        }
        ahb2.a(yz2, 1003, n2, n3, n4, 0);
        return true;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (ahb2.E) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl2 = ahb2.v(n2, n3, n4);
        if (bl2 || aji2.f()) {
            if (bl2 && !ala.b(n5)) {
                ahb2.a(n2, n3, n4, n5 | 4, 2);
                ahb2.a(null, 1003, n2, n3, n4, 0);
            } else if (!bl2 && ala.b(n5)) {
                ahb2.a(n2, n3, n4, n5 & 0xFFFFFFFB, 2);
                ahb2.a(null, 1003, n2, n3, n4, 0);
            }
        }
    }

    public static boolean b(int n2) {
        return (n2 & 4) != 0;
    }
}

