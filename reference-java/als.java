/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class als
extends aji {
    protected als() {
        super(awt.q);
        this.a(abt.c);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.b(ahl2.e(n2, n3, n4));
    }

    public void b(int n2) {
        int n3 = n2;
        float f2 = 0.125f;
        if (n3 == 2) {
            this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
        }
        if (n3 == 3) {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
        }
        if (n3 == 4) {
            this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        }
        if (n3 == 5) {
            this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
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
    public int b() {
        return 8;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2 - 1, n3, n4).r()) {
            return true;
        }
        if (ahb2.a(n2 + 1, n3, n4).r()) {
            return true;
        }
        if (ahb2.a(n2, n3, n4 - 1).r()) {
            return true;
        }
        return ahb2.a(n2, n3, n4 + 1).r();
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        int n7 = n6;
        if ((n7 == 0 || n5 == 2) && ahb2.a(n2, n3, n4 + 1).r()) {
            n7 = 2;
        }
        if ((n7 == 0 || n5 == 3) && ahb2.a(n2, n3, n4 - 1).r()) {
            n7 = 3;
        }
        if ((n7 == 0 || n5 == 4) && ahb2.a(n2 + 1, n3, n4).r()) {
            n7 = 4;
        }
        if ((n7 == 0 || n5 == 5) && ahb2.a(n2 - 1, n3, n4).r()) {
            n7 = 5;
        }
        return n7;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = ahb2.e(n2, n3, n4);
        boolean bl2 = false;
        if (n5 == 2 && ahb2.a(n2, n3, n4 + 1).r()) {
            bl2 = true;
        }
        if (n5 == 3 && ahb2.a(n2, n3, n4 - 1).r()) {
            bl2 = true;
        }
        if (n5 == 4 && ahb2.a(n2 + 1, n3, n4).r()) {
            bl2 = true;
        }
        if (n5 == 5 && ahb2.a(n2 - 1, n3, n4).r()) {
            bl2 = true;
        }
        if (!bl2) {
            this.b(ahb2, n2, n3, n4, n5, 0);
            ahb2.f(n2, n3, n4);
        }
        super.a(ahb2, n2, n3, n4, aji2);
    }

    @Override
    public int a(Random random) {
        return 1;
    }
}

