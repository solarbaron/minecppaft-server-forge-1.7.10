/*
 * Decompiled with CFR 0.152.
 */
public class aoi
extends aji {
    public static final String[] a = new String[]{"normal", "mossy"};

    public aoi(aji aji2) {
        super(aji2.J);
        this.c(aji2.v);
        this.b(aji2.w / 3.0f);
        this.a(aji2.H);
        this.a(abt.b);
    }

    @Override
    public int b() {
        return 32;
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
    public boolean c() {
        return false;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        boolean bl2 = this.e(ahl2, n2, n3, n4 - 1);
        boolean bl3 = this.e(ahl2, n2, n3, n4 + 1);
        boolean bl4 = this.e(ahl2, n2 - 1, n3, n4);
        boolean bl5 = this.e(ahl2, n2 + 1, n3, n4);
        float f2 = 0.25f;
        float f3 = 0.75f;
        float f4 = 0.25f;
        float f5 = 0.75f;
        float f6 = 1.0f;
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
        if (bl2 && bl3 && !bl4 && !bl5) {
            f6 = 0.8125f;
            f2 = 0.3125f;
            f3 = 0.6875f;
        } else if (!bl2 && !bl3 && bl4 && bl5) {
            f6 = 0.8125f;
            f4 = 0.3125f;
            f5 = 0.6875f;
        }
        this.a(f2, 0.0f, f4, f3, f6, f5);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        this.F = 1.5;
        return super.a(ahb2, n2, n3, n4);
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

    @Override
    public int a(int n2) {
        return n2;
    }
}

