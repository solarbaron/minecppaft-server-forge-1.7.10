/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public abstract class alw
extends aji {
    protected alw(awt awt2) {
        super(awt2);
        float f2 = 0.0f;
        float f3 = 0.0f;
        this.a(0.0f + f3, 0.0f + f2, 0.0f + f3, 1.0f + f3, 1.0f + f2, 1.0f + f3);
        this.a(true);
    }

    @Override
    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        return this.J != awt.i;
    }

    public static float b(int n2) {
        if (n2 >= 8) {
            n2 = 0;
        }
        return (float)(n2 + 1) / 9.0f;
    }

    protected int e(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4).o() == this.J) {
            return ahb2.e(n2, n3, n4);
        }
        return -1;
    }

    protected int e(ahl ahl2, int n2, int n3, int n4) {
        if (ahl2.a(n2, n3, n4).o() != this.J) {
            return -1;
        }
        int n5 = ahl2.e(n2, n3, n4);
        if (n5 >= 8) {
            n5 = 0;
        }
        return n5;
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
    public boolean a(int n2, boolean bl2) {
        return bl2 && n2 == 0;
    }

    @Override
    public boolean d(ahl ahl2, int n2, int n3, int n4, int n5) {
        awt awt2 = ahl2.a(n2, n3, n4).o();
        if (awt2 == this.J) {
            return false;
        }
        if (n5 == 1) {
            return true;
        }
        if (awt2 == awt.w) {
            return false;
        }
        return super.d(ahl2, n2, n3, n4, n5);
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return null;
    }

    @Override
    public int b() {
        return 4;
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        return null;
    }

    @Override
    public int a(Random random) {
        return 0;
    }

    private azw f(ahl ahl2, int n2, int n3, int n4) {
        int n5;
        azw azw2 = azw.a(0.0, 0.0, 0.0);
        int n6 = this.e(ahl2, n2, n3, n4);
        for (n5 = 0; n5 < 4; ++n5) {
            int n7;
            int n8;
            int n9 = n2;
            int n10 = n3;
            int n11 = n4;
            if (n5 == 0) {
                --n9;
            }
            if (n5 == 1) {
                --n11;
            }
            if (n5 == 2) {
                ++n9;
            }
            if (n5 == 3) {
                ++n11;
            }
            if ((n8 = this.e(ahl2, n9, n10, n11)) < 0) {
                if (ahl2.a(n9, n10, n11).o().c() || (n8 = this.e(ahl2, n9, n10 - 1, n11)) < 0) continue;
                n7 = n8 - (n6 - 8);
                azw2 = azw2.c((n9 - n2) * n7, (n10 - n3) * n7, (n11 - n4) * n7);
                continue;
            }
            if (n8 < 0) continue;
            n7 = n8 - n6;
            azw2 = azw2.c((n9 - n2) * n7, (n10 - n3) * n7, (n11 - n4) * n7);
        }
        if (ahl2.e(n2, n3, n4) >= 8) {
            n5 = 0;
            if (n5 != 0 || this.d(ahl2, n2, n3, n4 - 1, 2)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2, n3, n4 + 1, 3)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2 - 1, n3, n4, 4)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2 + 1, n3, n4, 5)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2, n3 + 1, n4 - 1, 2)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2, n3 + 1, n4 + 1, 3)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2 - 1, n3 + 1, n4, 4)) {
                n5 = 1;
            }
            if (n5 != 0 || this.d(ahl2, n2 + 1, n3 + 1, n4, 5)) {
                n5 = 1;
            }
            if (n5 != 0) {
                azw2 = azw2.a().c(0.0, -6.0, 0.0);
            }
        }
        azw2 = azw2.a();
        return azw2;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2, azw azw2) {
        azw azw3 = this.f((ahl)ahb2, n2, n3, n4);
        azw2.a += azw3.a;
        azw2.b += azw3.b;
        azw2.c += azw3.c;
    }

    @Override
    public int a(ahb ahb2) {
        if (this.J == awt.h) {
            return 5;
        }
        if (this.J == awt.i) {
            if (ahb2.t.g) {
                return 10;
            }
            return 30;
        }
        return 0;
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        this.n(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        this.n(ahb2, n2, n3, n4);
    }

    private void n(ahb ahb2, int n2, int n3, int n4) {
        if (ahb2.a(n2, n3, n4) != this) {
            return;
        }
        if (this.J == awt.i) {
            boolean bl2 = false;
            if (bl2 || ahb2.a(n2, n3, n4 - 1).o() == awt.h) {
                bl2 = true;
            }
            if (bl2 || ahb2.a(n2, n3, n4 + 1).o() == awt.h) {
                bl2 = true;
            }
            if (bl2 || ahb2.a(n2 - 1, n3, n4).o() == awt.h) {
                bl2 = true;
            }
            if (bl2 || ahb2.a(n2 + 1, n3, n4).o() == awt.h) {
                bl2 = true;
            }
            if (bl2 || ahb2.a(n2, n3 + 1, n4).o() == awt.h) {
                bl2 = true;
            }
            if (bl2) {
                int n5 = ahb2.e(n2, n3, n4);
                if (n5 == 0) {
                    ahb2.b(n2, n3, n4, ajn.Z);
                } else if (n5 <= 4) {
                    ahb2.b(n2, n3, n4, ajn.e);
                }
                this.m(ahb2, n2, n3, n4);
            }
        }
    }

    protected void m(ahb ahb2, int n2, int n3, int n4) {
        ahb2.a((float)n2 + 0.5f, (double)((float)n3 + 0.5f), (double)((float)n4 + 0.5f), "random.fizz", 0.5f, 2.6f + (ahb2.s.nextFloat() - ahb2.s.nextFloat()) * 0.8f);
        for (int i2 = 0; i2 < 8; ++i2) {
            ahb2.a("largesmoke", (double)n2 + Math.random(), (double)n3 + 1.2, (double)n4 + Math.random(), 0.0, 0.0, 0.0);
        }
    }
}

