/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class akn
extends aji {
    protected akn(awt awt2) {
        super(awt2);
        float f2 = 0.5f;
        float f3 = 1.0f;
        this.a(0.5f - f2, 0.0f, 0.5f - f2, 0.5f + f2, f3, 0.5f + f2);
    }

    @Override
    public boolean c() {
        return false;
    }

    @Override
    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        int n5 = this.g(ahl2, n2, n3, n4);
        return (n5 & 4) != 0;
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public int b() {
        return 7;
    }

    @Override
    public azt a(ahb ahb2, int n2, int n3, int n4) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.b(this.g(ahl2, n2, n3, n4));
    }

    public int e(ahl ahl2, int n2, int n3, int n4) {
        return this.g(ahl2, n2, n3, n4) & 3;
    }

    public boolean f(ahl ahl2, int n2, int n3, int n4) {
        return (this.g(ahl2, n2, n3, n4) & 4) != 0;
    }

    private void b(int n2) {
        boolean bl2;
        float f2 = 0.1875f;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f);
        int n3 = n2 & 3;
        boolean bl3 = (n2 & 4) != 0;
        boolean bl4 = bl2 = (n2 & 0x10) != 0;
        if (n3 == 0) {
            if (bl3) {
                if (!bl2) {
                    this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
                } else {
                    this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
                }
            } else {
                this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
            }
        } else if (n3 == 1) {
            if (bl3) {
                if (!bl2) {
                    this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
                } else {
                    this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
                }
            } else {
                this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
            }
        } else if (n3 == 2) {
            if (bl3) {
                if (!bl2) {
                    this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
                } else {
                    this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, f2);
                }
            } else {
                this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            }
        } else if (n3 == 3) {
            if (bl3) {
                if (!bl2) {
                    this.a(0.0f, 0.0f, 0.0f, f2, 1.0f, 1.0f);
                } else {
                    this.a(1.0f - f2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
                }
            } else {
                this.a(0.0f, 0.0f, 1.0f - f2, 1.0f, 1.0f, 1.0f);
            }
        }
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        if (this.J == awt.f) {
            return true;
        }
        int n6 = this.g(ahb2, n2, n3, n4);
        int n7 = n6 & 7;
        n7 ^= 4;
        if ((n6 & 8) == 0) {
            ahb2.a(n2, n3, n4, n7, 2);
            ahb2.c(n2, n3, n4, n2, n3, n4);
        } else {
            ahb2.a(n2, n3 - 1, n4, n7, 2);
            ahb2.c(n2, n3 - 1, n4, n2, n3, n4);
        }
        ahb2.a(yz2, 1003, n2, n3, n4, 0);
        return true;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        boolean bl3;
        int n5 = this.g(ahb2, n2, n3, n4);
        boolean bl4 = bl3 = (n5 & 4) != 0;
        if (bl3 == bl2) {
            return;
        }
        int n6 = n5 & 7;
        n6 ^= 4;
        if ((n5 & 8) == 0) {
            ahb2.a(n2, n3, n4, n6, 2);
            ahb2.c(n2, n3, n4, n2, n3, n4);
        } else {
            ahb2.a(n2, n3 - 1, n4, n6, 2);
            ahb2.c(n2, n3 - 1, n4, n2, n3, n4);
        }
        ahb2.a(null, 1003, n2, n3, n4, 0);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
        int n5 = ahb2.e(n2, n3, n4);
        if ((n5 & 8) == 0) {
            boolean bl2 = false;
            if (ahb2.a(n2, n3 + 1, n4) != this) {
                ahb2.f(n2, n3, n4);
                bl2 = true;
            }
            if (!ahb.a(ahb2, n2, n3 - 1, n4)) {
                ahb2.f(n2, n3, n4);
                bl2 = true;
                if (ahb2.a(n2, n3 + 1, n4) == this) {
                    ahb2.f(n2, n3 + 1, n4);
                }
            }
            if (bl2) {
                if (!ahb2.E) {
                    this.b(ahb2, n2, n3, n4, n5, 0);
                }
            } else {
                boolean bl3;
                boolean bl4 = bl3 = ahb2.v(n2, n3, n4) || ahb2.v(n2, n3 + 1, n4);
                if ((bl3 || aji2.f()) && aji2 != this) {
                    this.a(ahb2, n2, n3, n4, bl3);
                }
            }
        } else {
            if (ahb2.a(n2, n3 - 1, n4) != this) {
                ahb2.f(n2, n3, n4);
            }
            if (aji2 != this) {
                this.a(ahb2, n2, n3 - 1, n4, aji2);
            }
        }
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if ((n2 & 8) != 0) {
            return null;
        }
        if (this.J == awt.f) {
            return ade.aw;
        }
        return ade.aq;
    }

    @Override
    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        this.a((ahl)ahb2, n2, n3, n4);
        return super.a(ahb2, n2, n3, n4, azw2, azw3);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        if (n3 >= 255) {
            return false;
        }
        return ahb.a(ahb2, n2, n3 - 1, n4) && super.c(ahb2, n2, n3, n4) && super.c(ahb2, n2, n3 + 1, n4);
    }

    @Override
    public int h() {
        return 1;
    }

    public int g(ahl ahl2, int n2, int n3, int n4) {
        int n5;
        int n6;
        boolean bl2;
        int n7 = ahl2.e(n2, n3, n4);
        boolean bl3 = bl2 = (n7 & 8) != 0;
        if (bl2) {
            n6 = ahl2.e(n2, n3 - 1, n4);
            n5 = n7;
        } else {
            n6 = n7;
            n5 = ahl2.e(n2, n3 + 1, n4);
        }
        boolean bl4 = (n5 & 1) != 0;
        return n6 & 7 | (bl2 ? 8 : 0) | (bl4 ? 16 : 0);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        if (yz2.bE.d && (n5 & 8) != 0 && ahb2.a(n2, n3 - 1, n4) == this) {
            ahb2.f(n2, n3 - 1, n4);
        }
    }
}

