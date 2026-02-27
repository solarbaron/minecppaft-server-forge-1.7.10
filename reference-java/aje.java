/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class aje
extends aji {
    protected final boolean a;

    public static final boolean b_(ahb ahb2, int n2, int n3, int n4) {
        return aje.a(ahb2.a(n2, n3, n4));
    }

    public static final boolean a(aji aji2) {
        return aji2 == ajn.aq || aji2 == ajn.D || aji2 == ajn.E || aji2 == ajn.cc;
    }

    protected aje(boolean bl2) {
        super(awt.q);
        this.a = bl2;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
        this.a(abt.e);
    }

    public boolean e() {
        return this.a;
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4, azw2, azw3);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        if (n5 >= 2 && n5 <= 5) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.625f, 1.0f);
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
        }
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 9;
    }

    @Override
    public int a(Random random) {
        return 1;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return ahb.a(ahb2, n2, n3 - 1, n4);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (!ahb2.E) {
            this.a(ahb2, n2, n3, n4, true);
            if (this.a) {
                this.a(ahb2, n2, n3, n4, this);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5;
        if (ahb2.E) {
            return;
        }
        int n6 = n5 = ahb2.e(n2, n3, n4);
        if (this.a) {
            n6 &= 7;
        }
        boolean bl2 = false;
        if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
            bl2 = true;
        }
        if (n6 == 2 && !ahb.a(ahb2, n2 + 1, n3, n4)) {
            bl2 = true;
        }
        if (n6 == 3 && !ahb.a(ahb2, n2 - 1, n3, n4)) {
            bl2 = true;
        }
        if (n6 == 4 && !ahb.a(ahb2, n2, n3, n4 - 1)) {
            bl2 = true;
        }
        if (n6 == 5 && !ahb.a(ahb2, n2, n3, n4 + 1)) {
            bl2 = true;
        }
        if (bl2) {
            this.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            ahb2.f(n2, n3, n4);
        } else {
            this.a(ahb2, n2, n3, n4, n5, n6, aji2);
        }
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, int n5, int n6, aji aji2) {
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        if (ahb2.E) {
            return;
        }
        new ajf(this, ahb2, n2, n3, n4).a(ahb2.v(n2, n3, n4), bl2);
    }

    @Override
    public int h() {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        int n6 = n5;
        if (this.a) {
            n6 &= 7;
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
        if (n6 == 2 || n6 == 3 || n6 == 4 || n6 == 5) {
            ahb2.d(n2, n3 + 1, n4, aji2);
        }
        if (this.a) {
            ahb2.d(n2, n3, n4, aji2);
            ahb2.d(n2, n3 - 1, n4, aji2);
        }
    }
}

