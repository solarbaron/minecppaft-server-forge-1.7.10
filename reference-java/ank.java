/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ank
extends ajc {
    private Class a;
    private boolean b;

    protected ank(Class clazz, boolean bl2) {
        super(awt.d);
        this.b = bl2;
        this.a = clazz;
        float f2 = 0.25f;
        float f3 = 1.0f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f3, 0.5f + f2);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        if (this.b) {
            return;
        }
        int n5 = ahl2.e(n2, n3, n4);
        float f2 = 0.28125f;
        float f3 = 0.78125f;
        float f4 = 0.0f;
        float f5 = 1.0f;
        float f6 = 0.125f;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        if (n5 == 2) {
            this.a(f4, f2, 1.0f - f6, f5, f3, 1.0f);
        }
        if (n5 == 3) {
            this.a(f4, f2, 0.0f, f5, f3, f6);
        }
        if (n5 == 4) {
            this.a(1.0f - f6, f2, f4, 1.0f, f3, f5);
        }
        if (n5 == 5) {
            this.a(0.0f, f2, f4, f6, f3, f5);
        }
    }

    @Override
    public int b() {
        return -1;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        try {
            return (aor)this.a.newInstance();
        }
        catch (Exception exception) {
            throw new RuntimeException(exception);
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return ade.ap;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        boolean bl2 = false;
        if (this.b) {
            if (!ahb2.a(n2, n3 - 1, n4).o().a()) {
                bl2 = true;
            }
        } else {
            int n5 = ahb2.e(n2, n3, n4);
            bl2 = true;
            if (n5 == 2 && ahb2.a(n2, n3, n4 + 1).o().a()) {
                bl2 = false;
            }
            if (n5 == 3 && ahb2.a(n2, n3, n4 - 1).o().a()) {
                bl2 = false;
            }
            if (n5 == 4 && ahb2.a(n2 + 1, n3, n4).o().a()) {
                bl2 = false;
            }
            if (n5 == 5 && ahb2.a(n2 - 1, n3, n4).o().a()) {
                bl2 = false;
            }
        }
        if (bl2) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
        }
        super.a(ahb2, n2, n3, n4, aji2);
    }
}

