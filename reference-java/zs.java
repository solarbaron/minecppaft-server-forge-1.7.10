/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public abstract class zs {
    public List b = new ArrayList();
    public List c = new ArrayList();
    public int d;
    private int f = -1;
    private int g;
    private final Set h = new HashSet();
    protected List e = new ArrayList();
    private Set i = new HashSet();

    protected aay a(aay aay2) {
        aay2.g = this.c.size();
        this.c.add(aay2);
        this.b.add(null);
        return aay2;
    }

    public void a(aac aac2) {
        if (this.e.contains(aac2)) {
            throw new IllegalArgumentException("Listener already listening");
        }
        this.e.add(aac2);
        aac2.a(this, this.a());
        this.b();
    }

    public List a() {
        ArrayList<add> arrayList = new ArrayList<add>();
        for (int i2 = 0; i2 < this.c.size(); ++i2) {
            arrayList.add(((aay)this.c.get(i2)).d());
        }
        return arrayList;
    }

    public void b() {
        for (int i2 = 0; i2 < this.c.size(); ++i2) {
            add add2 = ((aay)this.c.get(i2)).d();
            add add3 = (add)this.b.get(i2);
            if (add.b(add3, add2)) continue;
            add3 = add2 == null ? null : add2.m();
            this.b.set(i2, add3);
            for (int i3 = 0; i3 < this.e.size(); ++i3) {
                ((aac)this.e.get(i3)).a(this, i2, add3);
            }
        }
    }

    public boolean a(yz yz2, int n2) {
        return false;
    }

    public aay a(rb rb2, int n2) {
        for (int i2 = 0; i2 < this.c.size(); ++i2) {
            aay aay2 = (aay)this.c.get(i2);
            if (!aay2.a(rb2, n2)) continue;
            return aay2;
        }
        return null;
    }

    public aay a(int n2) {
        return (aay)this.c.get(n2);
    }

    public add b(yz yz2, int n2) {
        aay aay2 = (aay)this.c.get(n2);
        if (aay2 != null) {
            return aay2.d();
        }
        return null;
    }

    public add a(int n2, int n3, int n4, yz yz2) {
        add add2 = null;
        yx yx2 = yz2.bm;
        if (n4 == 5) {
            int n5 = this.g;
            this.g = zs.c(n3);
            if ((n5 != 1 || this.g != 2) && n5 != this.g) {
                this.d();
            } else if (yx2.o() == null) {
                this.d();
            } else if (this.g == 0) {
                this.f = zs.b(n3);
                if (zs.d(this.f)) {
                    this.g = 1;
                    this.h.clear();
                } else {
                    this.d();
                }
            } else if (this.g == 1) {
                aay aay2 = (aay)this.c.get(n2);
                if (aay2 != null && zs.a(aay2, yx2.o(), true) && aay2.a(yx2.o()) && yx2.o().b > this.h.size() && this.b(aay2)) {
                    this.h.add(aay2);
                }
            } else if (this.g == 2) {
                if (!this.h.isEmpty()) {
                    add add3 = yx2.o().m();
                    int n6 = yx2.o().b;
                    for (aay aay3 : this.h) {
                        if (aay3 == null || !zs.a(aay3, yx2.o(), true) || !aay3.a(yx2.o()) || yx2.o().b < this.h.size() || !this.b(aay3)) continue;
                        add add4 = add3.m();
                        int n7 = aay3.e() ? aay3.d().b : 0;
                        zs.a(this.h, this.f, add4, n7);
                        if (add4.b > add4.e()) {
                            add4.b = add4.e();
                        }
                        if (add4.b > aay3.a()) {
                            add4.b = aay3.a();
                        }
                        n6 -= add4.b - n7;
                        aay3.c(add4);
                    }
                    add3.b = n6;
                    if (add3.b <= 0) {
                        add3 = null;
                    }
                    yx2.b(add3);
                }
                this.d();
            } else {
                this.d();
            }
        } else if (this.g != 0) {
            this.d();
        } else if (!(n4 != 0 && n4 != 1 || n3 != 0 && n3 != 1)) {
            if (n2 == -999) {
                if (yx2.o() != null && n2 == -999) {
                    if (n3 == 0) {
                        yz2.a(yx2.o(), true);
                        yx2.b((add)null);
                    }
                    if (n3 == 1) {
                        yz2.a(yx2.o().a(1), true);
                        if (yx2.o().b == 0) {
                            yx2.b((add)null);
                        }
                    }
                }
            } else if (n4 == 1) {
                add add5;
                if (n2 < 0) {
                    return null;
                }
                aay aay4 = (aay)this.c.get(n2);
                if (aay4 != null && aay4.a(yz2) && (add5 = this.b(yz2, n2)) != null) {
                    adb adb2 = add5.b();
                    add2 = add5.m();
                    if (aay4.d() != null && aay4.d().b() == adb2) {
                        this.a(n2, n3, true, yz2);
                    }
                }
            } else {
                if (n2 < 0) {
                    return null;
                }
                aay aay5 = (aay)this.c.get(n2);
                if (aay5 != null) {
                    add add6 = aay5.d();
                    add add7 = yx2.o();
                    if (add6 != null) {
                        add2 = add6.m();
                    }
                    if (add6 == null) {
                        if (add7 != null && aay5.a(add7)) {
                            int n8;
                            int n9 = n8 = n3 == 0 ? add7.b : 1;
                            if (n8 > aay5.a()) {
                                n8 = aay5.a();
                            }
                            if (add7.b >= n8) {
                                aay5.c(add7.a(n8));
                            }
                            if (add7.b == 0) {
                                yx2.b((add)null);
                            }
                        }
                    } else if (aay5.a(yz2)) {
                        int n10;
                        if (add7 == null) {
                            int n11 = n3 == 0 ? add6.b : (add6.b + 1) / 2;
                            add add8 = aay5.a(n11);
                            yx2.b(add8);
                            if (add6.b == 0) {
                                aay5.c(null);
                            }
                            aay5.a(yz2, yx2.o());
                        } else if (aay5.a(add7)) {
                            if (add6.b() != add7.b() || add6.k() != add7.k() || !add.a(add6, add7)) {
                                if (add7.b <= aay5.a()) {
                                    aay5.c(add7);
                                    yx2.b(add6);
                                }
                            } else {
                                int n12;
                                int n13 = n12 = n3 == 0 ? add7.b : 1;
                                if (n12 > aay5.a() - add6.b) {
                                    n12 = aay5.a() - add6.b;
                                }
                                if (n12 > add7.e() - add6.b) {
                                    n12 = add7.e() - add6.b;
                                }
                                add7.a(n12);
                                if (add7.b == 0) {
                                    yx2.b((add)null);
                                }
                                add6.b += n12;
                            }
                        } else if (add6.b() == add7.b() && add7.e() > 1 && (!add6.h() || add6.k() == add7.k()) && add.a(add6, add7) && (n10 = add6.b) > 0 && n10 + add7.b <= add7.e()) {
                            add7.b += n10;
                            add6 = aay5.a(n10);
                            if (add6.b == 0) {
                                aay5.c(null);
                            }
                            aay5.a(yz2, yx2.o());
                        }
                    }
                    aay5.f();
                }
            }
        } else if (n4 == 2 && n3 >= 0 && n3 < 9) {
            aay aay6 = (aay)this.c.get(n2);
            if (aay6.a(yz2)) {
                add add9 = yx2.a(n3);
                boolean bl2 = add9 == null || aay6.f == yx2 && aay6.a(add9);
                int n14 = -1;
                if (!bl2) {
                    n14 = yx2.j();
                    bl2 |= n14 > -1;
                }
                if (aay6.e() && bl2) {
                    add add10 = aay6.d();
                    yx2.a(n3, add10.m());
                    if (aay6.f == yx2 && aay6.a(add9) || add9 == null) {
                        aay6.a(add10.b);
                        aay6.c(add9);
                        aay6.a(yz2, add10);
                    } else if (n14 > -1) {
                        yx2.a(add9);
                        aay6.a(add10.b);
                        aay6.c(null);
                        aay6.a(yz2, add10);
                    }
                } else if (!aay6.e() && add9 != null && aay6.a(add9)) {
                    yx2.a(n3, null);
                    aay6.c(add9);
                }
            }
        } else if (n4 == 3 && yz2.bE.d && yx2.o() == null && n2 >= 0) {
            aay aay7 = (aay)this.c.get(n2);
            if (aay7 != null && aay7.e()) {
                add add11 = aay7.d().m();
                add11.b = add11.e();
                yx2.b(add11);
            }
        } else if (n4 == 4 && yx2.o() == null && n2 >= 0) {
            aay aay8 = (aay)this.c.get(n2);
            if (aay8 != null && aay8.e() && aay8.a(yz2)) {
                add add12 = aay8.a(n3 == 0 ? 1 : aay8.d().b);
                aay8.a(yz2, add12);
                yz2.a(add12, true);
            }
        } else if (n4 == 6 && n2 >= 0) {
            aay aay9 = (aay)this.c.get(n2);
            add add13 = yx2.o();
            if (!(add13 == null || aay9 != null && aay9.e() && aay9.a(yz2))) {
                int n15 = n3 == 0 ? 0 : this.c.size() - 1;
                int n16 = n3 == 0 ? 1 : -1;
                for (int i2 = 0; i2 < 2; ++i2) {
                    for (int i3 = n15; i3 >= 0 && i3 < this.c.size() && add13.b < add13.e(); i3 += n16) {
                        aay aay10 = (aay)this.c.get(i3);
                        if (!aay10.e() || !zs.a(aay10, add13, true) || !aay10.a(yz2) || !this.a(add13, aay10) || i2 == 0 && aay10.d().b == aay10.d().e()) continue;
                        int n17 = Math.min(add13.e() - add13.b, aay10.d().b);
                        add add14 = aay10.a(n17);
                        add13.b += n17;
                        if (add14.b <= 0) {
                            aay10.c(null);
                        }
                        aay10.a(yz2, add14);
                    }
                }
            }
            this.b();
        }
        return add2;
    }

    public boolean a(add add2, aay aay2) {
        return true;
    }

    protected void a(int n2, int n3, boolean bl2, yz yz2) {
        this.a(n2, n3, 1, yz2);
    }

    public void b(yz yz2) {
        yx yx2 = yz2.bm;
        if (yx2.o() != null) {
            yz2.a(yx2.o(), false);
            yx2.b((add)null);
        }
    }

    public void a(rb rb2) {
        this.b();
    }

    public void a(int n2, add add2) {
        this.a(n2).c(add2);
    }

    public boolean c(yz yz2) {
        return !this.i.contains(yz2);
    }

    public void a(yz yz2, boolean bl2) {
        if (bl2) {
            this.i.remove(yz2);
        } else {
            this.i.add(yz2);
        }
    }

    public abstract boolean a(yz var1);

    protected boolean a(add add2, int n2, int n3, boolean bl2) {
        add add3;
        aay aay2;
        boolean bl3 = false;
        int n4 = n2;
        if (bl2) {
            n4 = n3 - 1;
        }
        if (add2.f()) {
            while (add2.b > 0 && (!bl2 && n4 < n3 || bl2 && n4 >= n2)) {
                aay2 = (aay)this.c.get(n4);
                add3 = aay2.d();
                if (add3 != null && add3.b() == add2.b() && (!add2.h() || add2.k() == add3.k()) && add.a(add2, add3)) {
                    int n5 = add3.b + add2.b;
                    if (n5 <= add2.e()) {
                        add2.b = 0;
                        add3.b = n5;
                        aay2.f();
                        bl3 = true;
                    } else if (add3.b < add2.e()) {
                        add2.b -= add2.e() - add3.b;
                        add3.b = add2.e();
                        aay2.f();
                        bl3 = true;
                    }
                }
                if (bl2) {
                    --n4;
                    continue;
                }
                ++n4;
            }
        }
        if (add2.b > 0) {
            n4 = bl2 ? n3 - 1 : n2;
            while (!bl2 && n4 < n3 || bl2 && n4 >= n2) {
                aay2 = (aay)this.c.get(n4);
                add3 = aay2.d();
                if (add3 == null) {
                    aay2.c(add2.m());
                    aay2.f();
                    add2.b = 0;
                    bl3 = true;
                    break;
                }
                if (bl2) {
                    --n4;
                    continue;
                }
                ++n4;
            }
        }
        return bl3;
    }

    public static int b(int n2) {
        return n2 >> 2 & 3;
    }

    public static int c(int n2) {
        return n2 & 3;
    }

    public static boolean d(int n2) {
        return n2 == 0 || n2 == 1;
    }

    protected void d() {
        this.g = 0;
        this.h.clear();
    }

    public static boolean a(aay aay2, add add2, boolean bl2) {
        boolean bl3;
        boolean bl4 = bl3 = aay2 == null || !aay2.e();
        if (aay2 != null && aay2.e() && add2 != null && add2.a(aay2.d()) && add.a(aay2.d(), add2)) {
            bl3 |= aay2.d().b + (bl2 ? 0 : add2.b) <= add2.e();
        }
        return bl3;
    }

    public static void a(Set set, int n2, add add2, int n3) {
        switch (n2) {
            case 0: {
                add2.b = qh.d((float)add2.b / (float)set.size());
                break;
            }
            case 1: {
                add2.b = 1;
            }
        }
        add2.b += n3;
    }

    public boolean b(aay aay2) {
        return true;
    }

    public static int b(rb rb2) {
        if (rb2 == null) {
            return 0;
        }
        int n2 = 0;
        float f2 = 0.0f;
        for (int i2 = 0; i2 < rb2.a(); ++i2) {
            add add2 = rb2.a(i2);
            if (add2 == null) continue;
            f2 += (float)add2.b / (float)Math.min(rb2.d(), add2.e());
            ++n2;
        }
        return qh.d((f2 /= (float)rb2.a()) * 14.0f) + (n2 > 0 ? 1 : 0);
    }
}

