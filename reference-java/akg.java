/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akg
extends ajc {
    private rf[] a = new rf[2];

    public akg() {
        super(awt.d);
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.375f, 1.0f);
        this.a(abt.d);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.375f, 1.0f);
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        return ahl2.e(n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
    }

    public void e(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.t.g) {
            return;
        }
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = ahb2.b(ahn.a, n2, n3, n4) - ahb2.j;
        float f2 = ahb2.d(1.0f);
        f2 = f2 < (float)Math.PI ? (f2 += (0.0f - f2) * 0.2f) : (f2 += ((float)Math.PI * 2 - f2) * 0.2f);
        n6 = Math.round((float)n6 * qh.b(f2));
        if (n6 < 0) {
            n6 = 0;
        }
        if (n6 > 15) {
            n6 = 15;
        }
        if (n5 != n6) {
            ahb2.a(n2, n3, n4, n6, 3);
        }
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean f() {
        return true;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apa();
    }
}

