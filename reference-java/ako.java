/*
 * Decompiled with CFR 0.152.
 */
import java.util.Random;

public class ako
extends ajr
implements ajo {
    public static final String[] a = new String[]{"sunflower", "syringa", "grass", "fern", "rose", "paeonia"};

    public ako() {
        super(awt.k);
        this.c(0.0f);
        this.a(h);
        this.c("doublePlant");
    }

    @Override
    public int b() {
        return 40;
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    public int e(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        if (!ako.c(n5)) {
            return n5 & 7;
        }
        return ahl2.e(n2, n3 - 1, n4) & 7;
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return super.c(ahb2, n2, n3, n4) && ahb2.c(n2, n3 + 1, n4);
    }

    @Override
    protected void e(ahb ahb2, int n2, int n3, int n4) {
        if (!this.j(ahb2, n2, n3, n4)) {
            int n5 = ahb2.e(n2, n3, n4);
            if (!ako.c(n5)) {
                this.b(ahb2, n2, n3, n4, n5, 0);
                if (ahb2.a(n2, n3 + 1, n4) == this) {
                    ahb2.d(n2, n3 + 1, n4, ajn.a, 0, 2);
                }
            }
            ahb2.d(n2, n3, n4, ajn.a, 0, 2);
        }
    }

    @Override
    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        if (ako.c(n5)) {
            return ahb2.a(n2, n3 - 1, n4) == this;
        }
        return ahb2.a(n2, n3 + 1, n4) == this && super.j(ahb2, n2, n3, n4);
    }

    @Override
    public adb a(int n2, Random random, int n3) {
        if (ako.c(n2)) {
            return null;
        }
        int n4 = ako.d(n2);
        if (n4 == 3 || n4 == 2) {
            return null;
        }
        return adb.a(this);
    }

    @Override
    public int a(int n2) {
        if (ako.c(n2)) {
            return 0;
        }
        return n2 & 7;
    }

    public static boolean c(int n2) {
        return (n2 & 8) != 0;
    }

    public static int d(int n2) {
        return n2 & 7;
    }

    public void c(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        ahb2.d(n2, n3, n4, this, n5, n6);
        ahb2.d(n2, n3 + 1, n4, this, 8, n6);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = ((qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3) + 2) % 4;
        ahb2.d(n2, n3 + 1, n4, this, 8 | n5, 2);
    }

    @Override
    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E && yz2.bF() != null && yz2.bF().b() == ade.aZ && !ako.c(n5) && this.b(ahb2, n2, n3, n4, n5, yz2)) {
            return;
        }
        super.a(ahb2, yz2, n2, n3, n4, n5);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        if (ako.c(n5)) {
            if (ahb2.a(n2, n3 - 1, n4) == this) {
                if (!yz2.bE.d) {
                    int n6 = ahb2.e(n2, n3 - 1, n4);
                    int n7 = ako.d(n6);
                    if (n7 == 3 || n7 == 2) {
                        if (!ahb2.E && yz2.bF() != null && yz2.bF().b() == ade.aZ) {
                            this.b(ahb2, n2, n3, n4, n6, yz2);
                        }
                        ahb2.f(n2, n3 - 1, n4);
                    } else {
                        ahb2.a(n2, n3 - 1, n4, true);
                    }
                } else {
                    ahb2.f(n2, n3 - 1, n4);
                }
            }
        } else if (yz2.bE.d && ahb2.a(n2, n3 + 1, n4) == this) {
            ahb2.d(n2, n3 + 1, n4, ajn.a, 0, 2);
        }
        super.a(ahb2, n2, n3, n4, n5, yz2);
    }

    private boolean b(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
        int n6 = ako.d(n5);
        if (n6 == 3 || n6 == 2) {
            yz2.a(pp.C[aji.b(this)], 1);
            int n7 = 1;
            if (n6 == 3) {
                n7 = 2;
            }
            this.a(ahb2, n2, n3, n4, new add(ajn.H, 2, n7));
            return true;
        }
        return false;
    }

    @Override
    public int k(ahb ahb2, int n2, int n3, int n4) {
        int n5 = ahb2.e(n2, n3, n4);
        if (ako.c(n5)) {
            return ako.d(ahb2.e(n2, n3 - 1, n4));
        }
        return ako.d(n5);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, boolean bl2) {
        int n5 = this.e((ahl)ahb2, n2, n3, n4);
        return n5 != 2 && n5 != 3;
    }

    @Override
    public boolean a(ahb ahb2, Random random, int n2, int n3, int n4) {
        return true;
    }

    @Override
    public void b(ahb ahb2, Random random, int n2, int n3, int n4) {
        int n5 = this.e((ahl)ahb2, n2, n3, n4);
        this.a(ahb2, n2, n3, n4, new add(this, 1, n5));
    }
}

