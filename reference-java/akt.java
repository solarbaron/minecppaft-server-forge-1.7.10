/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class akt
extends ajc {
    public static boolean a;

    protected akt(awt awt2) {
        super(awt2);
        this.a(1.0f);
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        return new apo();
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        float f2 = 0.0625f;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, f2, 1.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
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
    public int a(Random random) {
        return 0;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        if (sa2.m == null && sa2.l == null && !ahb2.E) {
            sa2.b(1);
        }
    }

    @Override
    public int b() {
        return -1;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        if (a) {
            return;
        }
        if (ahb2.t.i != 0) {
            ahb2.f(n2, n3, n4);
        }
    }

    @Override
    public awv f(int n2) {
        return awv.J;
    }
}

