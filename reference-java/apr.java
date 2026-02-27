/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class apr
extends ajc {
    public apr() {
        super(awt.H);
        this.c(-1.0f);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return null;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        aor aor2 = ahb2.o(n2, n3, n4);
        if (aor2 instanceof aps) {
            ((aps)aor2).f();
        } else {
            super.a(ahb2, n2, n3, n4, aji2, n5);
        }
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return false;
    }

    @Override
    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
        return false;
    }

    @Override
    public int b() {
        return -1;
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
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (!ahb2.E && ahb2.o(n2, n3, n4) == null) {
            ahb2.f(n2, n3, n4);
            return true;
        }
        return false;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (ahb2.E) {
            return;
        }
        aps aps2 = this.e(ahb2, n2, n3, n4);
        if (aps2 == null) {
            return;
        }
        aps2.a().b(ahb2, n2, n3, n4, aps2.p(), 0);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        if (!ahb2.E) {
            ahb2.o(n2, n3, n4);
        }
    }

    public static aor a(aji aji2, int n2, int n3, boolean bl2, boolean bl3) {
        return new aps(aji2, n2, n3, bl2, bl3);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        aps aps2 = this.e(ahb2, n2, n3, n4);
        if (aps2 == null) {
            return null;
        }
        float f2 = aps2.a(0.0f);
        if (aps2.b()) {
            f2 = 1.0f - f2;
        }
        return this.a(ahb2, n2, n3, n4, aps2.a(), f2, aps2.c());
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        aps aps2 = this.e(ahl2, n2, n3, n4);
        if (aps2 != null) {
            aji aji2 = aps2.a();
            if (aji2 == this || aji2.o() == awt.a) {
                return;
            }
            aji2.a(ahl2, n2, n3, n4);
            float f2 = aps2.a(0.0f);
            if (aps2.b()) {
                f2 = 1.0f - f2;
            }
            int n5 = aps2.c();
            this.B = aji2.x() - (double)((float)q.b[n5] * f2);
            this.C = aji2.z() - (double)((float)q.c[n5] * f2);
            this.D = aji2.B() - (double)((float)q.d[n5] * f2);
            this.E = aji2.y() - (double)((float)q.b[n5] * f2);
            this.F = aji2.A() - (double)((float)q.c[n5] * f2);
            this.G = aji2.C() - (double)((float)q.d[n5] * f2);
        }
    }

    public azt a(ahb ahb2, int n2, int n3, int n4, aji aji2, float f2, int n5) {
        if (aji2 == this || aji2.o() == awt.a) {
            return null;
        }
        azt azt2 = aji2.a(ahb2, n2, n3, n4);
        if (azt2 == null) {
            return null;
        }
        if (q.b[n5] < 0) {
            azt2.a -= (double)((float)q.b[n5] * f2);
        } else {
            azt2.d -= (double)((float)q.b[n5] * f2);
        }
        if (q.c[n5] < 0) {
            azt2.b -= (double)((float)q.c[n5] * f2);
        } else {
            azt2.e -= (double)((float)q.c[n5] * f2);
        }
        if (q.d[n5] < 0) {
            azt2.c -= (double)((float)q.d[n5] * f2);
        } else {
            azt2.f -= (double)((float)q.d[n5] * f2);
        }
        return azt2;
    }

    private aps e(ahl ahl2, int n2, int n3, int n4) {
        aor aor2 = ahl2.o(n2, n3, n4);
        if (aor2 instanceof aps) {
            return (aps)aor2;
        }
        return null;
    }
}

