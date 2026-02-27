/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;

public class akz
extends aji {
    private final String a;

    public akz(String string, awt awt2) {
        super(awt2);
        this.a = string;
        this.a(abt.c);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        boolean bl2 = this.e(ahb2, n2, n3, n4 - 1);
        boolean bl3 = this.e(ahb2, n2, n3, n4 + 1);
        boolean bl4 = this.e(ahb2, n2 - 1, n3, n4);
        boolean bl5 = this.e(ahb2, n2 + 1, n3, n4);
        float f2 = 0.375f;
        float f3 = 0.625f;
        float f4 = 0.375f;
        float f5 = 0.625f;
        if (bl2) {
            f4 = 0.0f;
        }
        if (bl3) {
            f5 = 1.0f;
        }
        if (bl2 || bl3) {
            this.a(f2, 0.0f, f4, f3, 1.5f, f5);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
        f4 = 0.375f;
        f5 = 0.625f;
        if (bl4) {
            f2 = 0.0f;
        }
        if (bl5) {
            f3 = 1.0f;
        }
        if (bl4 || bl5 || !bl2 && !bl3) {
            this.a(f2, 0.0f, f4, f3, 1.5f, f5);
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
        if (bl2) {
            f4 = 0.0f;
        }
        if (bl3) {
            f5 = 1.0f;
        }
        this.a(f2, 0.0f, f4, f3, 1.0f, f5);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        boolean bl2 = this.e(ahl2, n2, n3, n4 - 1);
        boolean bl3 = this.e(ahl2, n2, n3, n4 + 1);
        boolean bl4 = this.e(ahl2, n2 - 1, n3, n4);
        boolean bl5 = this.e(ahl2, n2 + 1, n3, n4);
        float f2 = 0.375f;
        float f3 = 0.625f;
        float f4 = 0.375f;
        float f5 = 0.625f;
        if (bl2) {
            f4 = 0.0f;
        }
        if (bl3) {
            f5 = 1.0f;
        }
        if (bl4) {
            f2 = 0.0f;
        }
        if (bl5) {
            f3 = 1.0f;
        }
        this.a(f2, 0.0f, f4, f3, 1.0f, f5);
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
        return false;
    }

    @Override
    public int b() {
        return 11;
    }

    public boolean e(ahl ahl2, int n2, int n3, int n4) {
        aji aji2 = ahl2.a(n2, n3, n4);
        if (aji2 == this || aji2 == ajn.be) {
            return true;
        }
        if (aji2.J.k() && aji2.d()) {
            return aji2.J != awt.C;
        }
        return false;
    }

    public static boolean a(aji aji2) {
        return aji2 == ajn.aJ || aji2 == ajn.bk;
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        return adf.a(yz2, ahb2, n2, n3, n4);
    }
}

