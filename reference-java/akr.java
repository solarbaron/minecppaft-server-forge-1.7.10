/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akr
extends alw {
    int a;
    boolean[] b = new boolean[4];
    int[] M = new int[4];

    protected akr(awt awt2) {
        super(awt2);
    }

    private void n(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        ahb2.d(n2, n3, n4, aji.e(aji.b(this) + 1), n5, 2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        int n5;
        int n6 = this.e(ahb2, n2, n3, n4);
        int n7 = 1;
        if (this.J == awt.i && !ahb2.t.f) {
            n7 = 2;
        }
        boolean bl2 = true;
        int n8 = this.a(ahb2);
        if (n6 > 0) {
            int n9 = -100;
            this.a = 0;
            n9 = this.a(ahb2, n2 - 1, n3, n4, n9);
            n9 = this.a(ahb2, n2 + 1, n3, n4, n9);
            n9 = this.a(ahb2, n2, n3, n4 - 1, n9);
            n5 = (n9 = this.a(ahb2, n2, n3, n4 + 1, n9)) + n7;
            if (n5 >= 8 || n9 < 0) {
                n5 = -1;
            }
            if (this.e(ahb2, n2, n3 + 1, n4) >= 0) {
                int n10 = this.e(ahb2, n2, n3 + 1, n4);
                n5 = n10 >= 8 ? n10 : n10 + 8;
            }
            if (this.a >= 2 && this.J == awt.h) {
                if (ahb2.a(n2, n3 - 1, n4).o().a()) {
                    n5 = 0;
                } else if (ahb2.a(n2, n3 - 1, n4).o() == this.J && ahb2.e(n2, n3 - 1, n4) == 0) {
                    n5 = 0;
                }
            }
            if (this.J == awt.i && n6 < 8 && n5 < 8 && n5 > n6 && random.nextInt(4) != 0) {
                n8 *= 4;
            }
            if (n5 == n6) {
                if (bl2) {
                    this.n(ahb2, n2, n3, n4);
                }
            } else {
                n6 = n5;
                if (n6 < 0) {
                    ahb2.f(n2, n3, n4);
                } else {
                    ahb2.a(n2, n3, n4, n6, 2);
                    ahb2.a(n2, n3, n4, this, n8);
                    ahb2.d(n2, n3, n4, this);
                }
            }
        } else {
            this.n(ahb2, n2, n3, n4);
        }
        if (this.q(ahb2, n2, n3 - 1, n4)) {
            if (this.J == awt.i && ahb2.a(n2, n3 - 1, n4).o() == awt.h) {
                ahb2.b(n2, n3 - 1, n4, ajn.b);
                this.m(ahb2, n2, n3 - 1, n4);
                return;
            }
            if (n6 >= 8) {
                this.h(ahb2, n2, n3 - 1, n4, n6);
            } else {
                this.h(ahb2, n2, n3 - 1, n4, n6 + 8);
            }
        } else if (n6 >= 0 && (n6 == 0 || this.p(ahb2, n2, n3 - 1, n4))) {
            boolean[] blArray = this.o(ahb2, n2, n3, n4);
            n5 = n6 + n7;
            if (n6 >= 8) {
                n5 = 1;
            }
            if (n5 >= 8) {
                return;
            }
            if (blArray[0]) {
                this.h(ahb2, n2 - 1, n3, n4, n5);
            }
            if (blArray[1]) {
                this.h(ahb2, n2 + 1, n3, n4, n5);
            }
            if (blArray[2]) {
                this.h(ahb2, n2, n3, n4 - 1, n5);
            }
            if (blArray[3]) {
                this.h(ahb2, n2, n3, n4 + 1, n5);
            }
        }
    }

    private void h(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (this.q(ahb2, n2, n3, n4)) {
            aji aji2 = ahb2.a(n2, n3, n4);
            if (this.J == awt.i) {
                this.m(ahb2, n2, n3, n4);
            } else {
                aji2.b(ahb2, n2, n3, n4, ahb2.e(n2, n3, n4), 0);
            }
            ahb2.d(n2, n3, n4, this, n5, 3);
        }
    }

    private int c(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        int n7 = 1000;
        for (int i2 = 0; i2 < 4; ++i2) {
            if (i2 == 0 && n6 == 1 || i2 == 1 && n6 == 0 || i2 == 2 && n6 == 3 || i2 == 3 && n6 == 2) continue;
            int n8 = n2;
            int n9 = n3;
            int n10 = n4;
            if (i2 == 0) {
                --n8;
            }
            if (i2 == 1) {
                ++n8;
            }
            if (i2 == 2) {
                --n10;
            }
            if (i2 == 3) {
                ++n10;
            }
            if (this.p(ahb2, n8, n9, n10) || ahb2.a(n8, n9, n10).o() == this.J && ahb2.e(n8, n9, n10) == 0) continue;
            if (this.p(ahb2, n8, n9 - 1, n10)) {
                int n11;
                if (n5 >= 4 || (n11 = this.c(ahb2, n8, n9, n10, n5 + 1, i2)) >= n7) continue;
                n7 = n11;
                continue;
            }
            return n5;
        }
        return n7;
    }

    private boolean[] o(ahb ahb2, int n2, int n3, int n4) {
        int n5;
        int n6;
        for (n6 = 0; n6 < 4; ++n6) {
            this.M[n6] = 1000;
            n5 = n2;
            int n7 = n3;
            int n8 = n4;
            if (n6 == 0) {
                --n5;
            }
            if (n6 == 1) {
                ++n5;
            }
            if (n6 == 2) {
                --n8;
            }
            if (n6 == 3) {
                ++n8;
            }
            if (this.p(ahb2, n5, n7, n8) || ahb2.a(n5, n7, n8).o() == this.J && ahb2.e(n5, n7, n8) == 0) continue;
            this.M[n6] = this.p(ahb2, n5, n7 - 1, n8) ? this.c(ahb2, n5, n7, n8, 1, n6) : 0;
        }
        n6 = this.M[0];
        for (n5 = 1; n5 < 4; ++n5) {
            if (this.M[n5] >= n6) continue;
            n6 = this.M[n5];
        }
        for (n5 = 0; n5 < 4; ++n5) {
            this.b[n5] = this.M[n5] == n6;
        }
        return this.b;
    }

    private boolean p(ahb ahb2, int n2, int n3, int n4) {
        aji aji2 = ahb2.a(n2, n3, n4);
        if (aji2 == ajn.ao || aji2 == ajn.av || aji2 == ajn.an || aji2 == ajn.ap || aji2 == ajn.aH) {
            return true;
        }
        if (aji2.J == awt.E) {
            return true;
        }
        return aji2.J.c();
    }

    protected int a(ahb ahb2, int n2, int n3, int n4, int n5) {
        int n6 = this.e(ahb2, n2, n3, n4);
        if (n6 < 0) {
            return n5;
        }
        if (n6 == 0) {
            ++this.a;
        }
        if (n6 >= 8) {
            n6 = 0;
        }
        return n5 < 0 || n6 < n5 ? n6 : n5;
    }

    private boolean q(ahb ahb2, int n2, int n3, int n4) {
        awt awt2 = ahb2.a(n2, n3, n4).o();
        if (awt2 == this.J) {
            return false;
        }
        if (awt2 == awt.i) {
            return false;
        }
        return !this.p(ahb2, n2, n3, n4);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        super.b(ahb2, n2, n3, n4);
        if (ahb2.a(n2, n3, n4) == this) {
            ahb2.a(n2, n3, n4, this, this.a(ahb2));
        }
    }

    @Override
    public boolean L() {
        return true;
    }
}

