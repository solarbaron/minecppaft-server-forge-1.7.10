/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aag
extends zs {
    public rb a = new aah(this, "Enchant", true, 1);
    private ahb h;
    private int i;
    private int j;
    private int k;
    private Random l = new Random();
    public long f;
    public int[] g = new int[3];

    public aag(yx yx2, ahb ahb2, int n2, int n3, int n4) {
        int n5;
        this.h = ahb2;
        this.i = n2;
        this.j = n3;
        this.k = n4;
        this.a(new aai(this, this.a, 0, 25, 47));
        for (n5 = 0; n5 < 3; ++n5) {
            for (int i2 = 0; i2 < 9; ++i2) {
                this.a(new aay(yx2, i2 + n5 * 9 + 9, 8 + i2 * 18, 84 + n5 * 18));
            }
        }
        for (n5 = 0; n5 < 9; ++n5) {
            this.a(new aay(yx2, n5, 8 + n5 * 18, 142));
        }
    }

    @Override
    public void a(aac aac2) {
        super.a(aac2);
        aac2.a((zs)this, 0, this.g[0]);
        aac2.a((zs)this, 1, this.g[1]);
        aac2.a((zs)this, 2, this.g[2]);
    }

    @Override
    public void b() {
        super.b();
        for (int i2 = 0; i2 < this.e.size(); ++i2) {
            aac aac2 = (aac)this.e.get(i2);
            aac2.a((zs)this, 0, this.g[0]);
            aac2.a((zs)this, 1, this.g[1]);
            aac2.a((zs)this, 2, this.g[2]);
        }
    }

    @Override
    public void a(rb rb2) {
        if (rb2 == this.a) {
            add add2 = rb2.a(0);
            if (add2 == null || !add2.x()) {
                for (int i2 = 0; i2 < 3; ++i2) {
                    this.g[i2] = 0;
                }
            } else {
                this.f = this.l.nextLong();
                if (!this.h.E) {
                    int n2;
                    int n3 = 0;
                    for (n2 = -1; n2 <= 1; ++n2) {
                        for (int i3 = -1; i3 <= 1; ++i3) {
                            if (n2 == 0 && i3 == 0 || !this.h.c(this.i + i3, this.j, this.k + n2) || !this.h.c(this.i + i3, this.j + 1, this.k + n2)) continue;
                            if (this.h.a(this.i + i3 * 2, this.j, this.k + n2 * 2) == ajn.X) {
                                ++n3;
                            }
                            if (this.h.a(this.i + i3 * 2, this.j + 1, this.k + n2 * 2) == ajn.X) {
                                ++n3;
                            }
                            if (i3 == 0 || n2 == 0) continue;
                            if (this.h.a(this.i + i3 * 2, this.j, this.k + n2) == ajn.X) {
                                ++n3;
                            }
                            if (this.h.a(this.i + i3 * 2, this.j + 1, this.k + n2) == ajn.X) {
                                ++n3;
                            }
                            if (this.h.a(this.i + i3, this.j, this.k + n2 * 2) == ajn.X) {
                                ++n3;
                            }
                            if (this.h.a(this.i + i3, this.j + 1, this.k + n2 * 2) != ajn.X) continue;
                            ++n3;
                        }
                    }
                    for (n2 = 0; n2 < 3; ++n2) {
                        this.g[n2] = afv.a(this.l, n2, n3, add2);
                    }
                    this.b();
                }
            }
        }
    }

    @Override
    public boolean a(yz yz2, int n2) {
        add add2 = this.a.a(0);
        if (this.g[n2] > 0 && add2 != null && (yz2.bF >= this.g[n2] || yz2.bE.d)) {
            if (!this.h.E) {
                boolean bl2;
                List list = afv.b(this.l, add2, this.g[n2]);
                boolean bl3 = bl2 = add2.b() == ade.aG;
                if (list != null) {
                    yz2.a(-this.g[n2]);
                    if (bl2) {
                        add2.a(ade.bR);
                    }
                    int n3 = bl2 && list.size() > 1 ? this.l.nextInt(list.size()) : -1;
                    for (int i2 = 0; i2 < list.size(); ++i2) {
                        agc agc2 = (agc)list.get(i2);
                        if (bl2 && i2 == n3) continue;
                        if (bl2) {
                            ade.bR.a(add2, agc2);
                            continue;
                        }
                        add2.a(agc2.b, agc2.c);
                    }
                    this.a(this.a);
                }
            }
            return true;
        }
        return false;
    }

    @Override
    public void b(yz yz2) {
        super.b(yz2);
        if (this.h.E) {
            return;
        }
        add add2 = this.a.a_(0);
        if (add2 != null) {
            yz2.a(add2, false);
        }
    }

    @Override
    public boolean a(yz yz2) {
        if (this.h.a(this.i, this.j, this.k) != ajn.bn) {
            return false;
        }
        return !(yz2.e((double)this.i + 0.5, (double)this.j + 0.5, (double)this.k + 0.5) > 64.0);
    }

    @Override
    public add b(yz yz2, int n2) {
        add add2 = null;
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null && aay2.e()) {
            add add3 = aay2.d();
            add2 = add3.m();
            if (n2 == 0) {
                if (!this.a(add3, 1, 37, true)) {
                    return null;
                }
            } else if (!((aay)this.c.get(0)).e() && ((aay)this.c.get(0)).a(add3)) {
                if (add3.p() && add3.b == 1) {
                    ((aay)this.c.get(0)).c(add3.m());
                    add3.b = 0;
                } else if (add3.b >= 1) {
                    ((aay)this.c.get(0)).c(new add(add3.b(), 1, add3.k()));
                    --add3.b;
                }
            } else {
                return null;
            }
            if (add3.b == 0) {
                aay2.c(null);
            } else {
                aay2.f();
            }
            if (add3.b == add2.b) {
                return null;
            }
            aay2.a(yz2, add3);
        }
        return add2;
    }
}

