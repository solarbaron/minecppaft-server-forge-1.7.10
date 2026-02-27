/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ajx
extends ajc {
    private final Random b = new Random();
    public final int a;

    protected ajx(int n2) {
        super(awt.d);
        this.a = n2;
        this.a(abt.c);
        this.a(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
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
        return 22;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        if (ahl2.a(n2, n3, n4 - 1) == this) {
            this.a(0.0625f, 0.0f, 0.0f, 0.9375f, 0.875f, 0.9375f);
        } else if (ahl2.a(n2, n3, n4 + 1) == this) {
            this.a(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 1.0f);
        } else if (ahl2.a(n2 - 1, n3, n4) == this) {
            this.a(0.0f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
        } else if (ahl2.a(n2 + 1, n3, n4) == this) {
            this.a(0.0625f, 0.0f, 0.0625f, 1.0f, 0.875f, 0.9375f);
        } else {
            this.a(0.0625f, 0.0f, 0.0625f, 0.9375f, 0.875f, 0.9375f);
        }
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        this.e(ahb2, n2, n3, n4);
        aji aji2 = ahb2.a(n2, n3, n4 - 1);
        aji aji3 = ahb2.a(n2, n3, n4 + 1);
        aji aji4 = ahb2.a(n2 - 1, n3, n4);
        aji aji5 = ahb2.a(n2 + 1, n3, n4);
        if (aji2 == this) {
            this.e(ahb2, n2, n3, n4 - 1);
        }
        if (aji3 == this) {
            this.e(ahb2, n2, n3, n4 + 1);
        }
        if (aji4 == this) {
            this.e(ahb2, n2 - 1, n3, n4);
        }
        if (aji5 == this) {
            this.e(ahb2, n2 + 1, n3, n4);
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        aji aji2 = ahb2.a(n2, n3, n4 - 1);
        aji aji3 = ahb2.a(n2, n3, n4 + 1);
        aji aji4 = ahb2.a(n2 - 1, n3, n4);
        aji aji5 = ahb2.a(n2 + 1, n3, n4);
        int n5 = 0;
        int n6 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3;
        if (n6 == 0) {
            n5 = 2;
        }
        if (n6 == 1) {
            n5 = 5;
        }
        if (n6 == 2) {
            n5 = 3;
        }
        if (n6 == 3) {
            n5 = 4;
        }
        if (aji2 != this && aji3 != this && aji4 != this && aji5 != this) {
            ahb2.a(n2, n3, n4, n5, 3);
        } else {
            if (!(aji2 != this && aji3 != this || n5 != 4 && n5 != 5)) {
                if (aji2 == this) {
                    ahb2.a(n2, n3, n4 - 1, n5, 3);
                } else {
                    ahb2.a(n2, n3, n4 + 1, n5, 3);
                }
                ahb2.a(n2, n3, n4, n5, 3);
            }
            if (!(aji4 != this && aji5 != this || n5 != 2 && n5 != 3)) {
                if (aji4 == this) {
                    ahb2.a(n2 - 1, n3, n4, n5, 3);
                } else {
                    ahb2.a(n2 + 1, n3, n4, n5, 3);
                }
                ahb2.a(n2, n3, n4, n5, 3);
            }
        }
        if (add2.u()) {
            ((aow)ahb2.o(n2, n3, n4)).a(add2.s());
        }
    }

    public void e(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.E) {
            return;
        }
        aji aji2 = ahb2.a(n2, n3, n4 - 1);
        aji aji3 = ahb2.a(n2, n3, n4 + 1);
        aji aji4 = ahb2.a(n2 - 1, n3, n4);
        aji aji5 = ahb2.a(n2 + 1, n3, n4);
        int n5 = 4;
        if (aji2 == this || aji3 == this) {
            int n6 = aji2 == this ? n4 - 1 : n4 + 1;
            aji aji6 = ahb2.a(n2 - 1, n3, n6);
            int n7 = aji2 == this ? n4 - 1 : n4 + 1;
            aji aji7 = ahb2.a(n2 + 1, n3, n7);
            n5 = 5;
            int n8 = -1;
            n8 = aji2 == this ? ahb2.e(n2, n3, n4 - 1) : ahb2.e(n2, n3, n4 + 1);
            if (n8 == 4) {
                n5 = 4;
            }
            if ((aji4.j() || aji6.j()) && !aji5.j() && !aji7.j()) {
                n5 = 5;
            }
            if ((aji5.j() || aji7.j()) && !aji4.j() && !aji6.j()) {
                n5 = 4;
            }
        } else if (aji4 == this || aji5 == this) {
            int n9 = aji4 == this ? n2 - 1 : n2 + 1;
            aji aji8 = ahb2.a(n9, n3, n4 - 1);
            int n10 = aji4 == this ? n2 - 1 : n2 + 1;
            aji aji9 = ahb2.a(n10, n3, n4 + 1);
            n5 = 3;
            int n11 = -1;
            n11 = aji4 == this ? ahb2.e(n2 - 1, n3, n4) : ahb2.e(n2 + 1, n3, n4);
            if (n11 == 2) {
                n5 = 2;
            }
            if ((aji2.j() || aji8.j()) && !aji3.j() && !aji9.j()) {
                n5 = 3;
            }
            if ((aji3.j() || aji9.j()) && !aji2.j() && !aji8.j()) {
                n5 = 2;
            }
        } else {
            n5 = 3;
            if (aji2.j() && !aji3.j()) {
                n5 = 3;
            }
            if (aji3.j() && !aji2.j()) {
                n5 = 2;
            }
            if (aji4.j() && !aji5.j()) {
                n5 = 5;
            }
            if (aji5.j() && !aji4.j()) {
                n5 = 4;
            }
        }
        ahb2.a(n2, n3, n4, n5, 3);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        int n5 = 0;
        if (ahb2.a(n2 - 1, n3, n4) == this) {
            ++n5;
        }
        if (ahb2.a(n2 + 1, n3, n4) == this) {
            ++n5;
        }
        if (ahb2.a(n2, n3, n4 - 1) == this) {
            ++n5;
        }
        if (ahb2.a(n2, n3, n4 + 1) == this) {
            ++n5;
        }
        if (n5 > 1) {
            return false;
        }
        if (this.n(ahb2, n2 - 1, n3, n4)) {
            return false;
        }
        if (this.n(ahb2, n2 + 1, n3, n4)) {
            return false;
        }
        if (this.n(ahb2, n2, n3, n4 - 1)) {
            return false;
        }
        return !this.n(ahb2, n2, n3, n4 + 1);
    }

    private boolean n(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4) != this) {
            return false;
        }
        if (ahb2.a(n2 - 1, n3, n4) == this) {
            return true;
        }
        if (ahb2.a(n2 + 1, n3, n4) == this) {
            return true;
        }
        if (ahb2.a(n2, n3, n4 - 1) == this) {
            return true;
        }
        return ahb2.a(n2, n3, n4 + 1) == this;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        super.a(ahb2, n2, n3, n4, aji2);
        aow aow2 = (aow)ahb2.o(n2, n3, n4);
        if (aow2 != null) {
            aow2.u();
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        aow aow2 = (aow)ahb2.o(n2, n3, n4);
        if (aow2 != null) {
            for (int i2 = 0; i2 < aow2.a(); ++i2) {
                add add2 = aow2.a(i2);
                if (add2 == null) continue;
                float f2 = this.b.nextFloat() * 0.8f + 0.1f;
                float f3 = this.b.nextFloat() * 0.8f + 0.1f;
                float f4 = this.b.nextFloat() * 0.8f + 0.1f;
                while (add2.b > 0) {
                    int n6 = this.b.nextInt(21) + 10;
                    if (n6 > add2.b) {
                        n6 = add2.b;
                    }
                    add2.b -= n6;
                    xk xk2 = new xk(ahb2, (float)n2 + f2, (float)n3 + f3, (float)n4 + f4, new add(add2.b(), n6, add2.k()));
                    float f5 = 0.05f;
                    xk2.v = (float)this.b.nextGaussian() * f5;
                    xk2.w = (float)this.b.nextGaussian() * f5 + 0.2f;
                    xk2.x = (float)this.b.nextGaussian() * f5;
                    if (add2.p()) {
                        xk2.f().d((dh)add2.q().b());
                    }
                    ahb2.d(xk2);
                }
            }
            ahb2.f(n2, n3, n4, aji2);
        }
        super.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (ahb2.E) {
            return true;
        }
        rb rb2 = this.m(ahb2, n2, n3, n4);
        if (rb2 != null) {
            yz2.a(rb2);
        }
        return true;
    }

    public rb m(ahb ahb2, int n2, int n3, int n4) {
        rb rb2 = (aow)ahb2.o(n2, n3, n4);
        if (rb2 == null) {
            return null;
        }
        if (ahb2.a(n2, n3 + 1, n4).r()) {
            return null;
        }
        if (ajx.o(ahb2, n2, n3, n4)) {
            return null;
        }
        if (ahb2.a(n2 - 1, n3, n4) == this && (ahb2.a(n2 - 1, n3 + 1, n4).r() || ajx.o(ahb2, n2 - 1, n3, n4))) {
            return null;
        }
        if (ahb2.a(n2 + 1, n3, n4) == this && (ahb2.a(n2 + 1, n3 + 1, n4).r() || ajx.o(ahb2, n2 + 1, n3, n4))) {
            return null;
        }
        if (ahb2.a(n2, n3, n4 - 1) == this && (ahb2.a(n2, n3 + 1, n4 - 1).r() || ajx.o(ahb2, n2, n3, n4 - 1))) {
            return null;
        }
        if (ahb2.a(n2, n3, n4 + 1) == this && (ahb2.a(n2, n3 + 1, n4 + 1).r() || ajx.o(ahb2, n2, n3, n4 + 1))) {
            return null;
        }
        if (ahb2.a(n2 - 1, n3, n4) == this) {
            rb2 = new ra("container.chestDouble", (aow)ahb2.o(n2 - 1, n3, n4), rb2);
        }
        if (ahb2.a(n2 + 1, n3, n4) == this) {
            rb2 = new ra("container.chestDouble", rb2, (aow)ahb2.o(n2 + 1, n3, n4));
        }
        if (ahb2.a(n2, n3, n4 - 1) == this) {
            rb2 = new ra("container.chestDouble", (aow)ahb2.o(n2, n3, n4 - 1), rb2);
        }
        if (ahb2.a(n2, n3, n4 + 1) == this) {
            rb2 = new ra("container.chestDouble", rb2, (aow)ahb2.o(n2, n3, n4 + 1));
        }
        return rb2;
    }

    @Override
    public aor a(ahb ahb2, int n2) {
        aow aow2 = new aow();
        return aow2;
    }

    @Override
    public boolean f() {
        return this.a == 1;
    }

    @Override
    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (!this.f()) {
            return 0;
        }
        int n6 = ((aow)ahl2.o((int)n2, (int)n3, (int)n4)).o;
        return qh.a(n6, 0, 15);
    }

    @Override
    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        if (n5 == 1) {
            return this.b(ahl2, n2, n3, n4, n5);
        }
        return 0;
    }

    private static boolean o(ahb ahb2, int n2, int n3, int n4) {
        for (sa sa2 : ahb2.a(wn.class, azt.a(n2, n3 + 1, n4, n2 + 1, n3 + 2, n4 + 1))) {
            wn wn2 = (wn)sa2;
            if (!wn2.ca()) continue;
            return true;
        }
        return false;
    }

    @Override
    public boolean M() {
        return true;
    }

    @Override
    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        return zs.b(this.m(ahb2, n2, n3, n4));
    }
}

