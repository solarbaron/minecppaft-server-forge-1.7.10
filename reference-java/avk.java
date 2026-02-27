/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public abstract class avk {
    protected asv f;
    protected int g;
    protected int h;

    public avk() {
    }

    protected avk(int n2) {
        this.h = n2;
        this.g = -1;
    }

    public dh b() {
        dh dh2 = new dh();
        dh2.a("id", avi.a(this));
        dh2.a("BB", this.f.h());
        dh2.a("O", this.g);
        dh2.a("GD", this.h);
        this.a(dh2);
        return dh2;
    }

    protected abstract void a(dh var1);

    public void a(ahb ahb2, dh dh2) {
        if (dh2.c("BB")) {
            this.f = new asv(dh2.l("BB"));
        }
        this.g = dh2.f("O");
        this.h = dh2.f("GD");
        this.b(dh2);
    }

    protected abstract void b(dh var1);

    public void a(avk avk2, List list, Random random) {
    }

    public abstract boolean a(ahb var1, Random var2, asv var3);

    public asv c() {
        return this.f;
    }

    public int d() {
        return this.h;
    }

    public static avk a(List list, asv asv2) {
        for (avk avk2 : list) {
            if (avk2.c() == null || !avk2.c().a(asv2)) continue;
            return avk2;
        }
        return null;
    }

    public agt a() {
        return new agt(this.f.e(), this.f.f(), this.f.g());
    }

    protected boolean a(ahb ahb2, asv asv2) {
        int n2;
        int n3;
        int n4 = Math.max(this.f.a - 1, asv2.a);
        int n5 = Math.max(this.f.b - 1, asv2.b);
        int n6 = Math.max(this.f.c - 1, asv2.c);
        int n7 = Math.min(this.f.d + 1, asv2.d);
        int n8 = Math.min(this.f.e + 1, asv2.e);
        int n9 = Math.min(this.f.f + 1, asv2.f);
        for (n3 = n4; n3 <= n7; ++n3) {
            for (n2 = n6; n2 <= n9; ++n2) {
                if (ahb2.a(n3, n5, n2).o().d()) {
                    return true;
                }
                if (!ahb2.a(n3, n8, n2).o().d()) continue;
                return true;
            }
        }
        for (n3 = n4; n3 <= n7; ++n3) {
            for (n2 = n5; n2 <= n8; ++n2) {
                if (ahb2.a(n3, n2, n6).o().d()) {
                    return true;
                }
                if (!ahb2.a(n3, n2, n9).o().d()) continue;
                return true;
            }
        }
        for (n3 = n6; n3 <= n9; ++n3) {
            for (n2 = n5; n2 <= n8; ++n2) {
                if (ahb2.a(n4, n2, n3).o().d()) {
                    return true;
                }
                if (!ahb2.a(n7, n2, n3).o().d()) continue;
                return true;
            }
        }
        return false;
    }

    protected int a(int n2, int n3) {
        switch (this.g) {
            case 0: 
            case 2: {
                return this.f.a + n2;
            }
            case 1: {
                return this.f.d - n3;
            }
            case 3: {
                return this.f.a + n3;
            }
        }
        return n2;
    }

    protected int a(int n2) {
        if (this.g == -1) {
            return n2;
        }
        return n2 + this.f.b;
    }

    protected int b(int n2, int n3) {
        switch (this.g) {
            case 2: {
                return this.f.f - n3;
            }
            case 0: {
                return this.f.c + n3;
            }
            case 1: 
            case 3: {
                return this.f.c + n2;
            }
        }
        return n3;
    }

    protected int a(aji aji2, int n2) {
        if (aji2 == ajn.aq) {
            if (this.g == 1 || this.g == 3) {
                if (n2 == 1) {
                    return 0;
                }
                return 1;
            }
        } else if (aji2 == ajn.ao || aji2 == ajn.av) {
            if (this.g == 0) {
                if (n2 == 0) {
                    return 2;
                }
                if (n2 == 2) {
                    return 0;
                }
            } else {
                if (this.g == 1) {
                    return n2 + 1 & 3;
                }
                if (this.g == 3) {
                    return n2 + 3 & 3;
                }
            }
        } else if (aji2 == ajn.ar || aji2 == ajn.ad || aji2 == ajn.bl || aji2 == ajn.bg || aji2 == ajn.bz) {
            if (this.g == 0) {
                if (n2 == 2) {
                    return 3;
                }
                if (n2 == 3) {
                    return 2;
                }
            } else if (this.g == 1) {
                if (n2 == 0) {
                    return 2;
                }
                if (n2 == 1) {
                    return 3;
                }
                if (n2 == 2) {
                    return 0;
                }
                if (n2 == 3) {
                    return 1;
                }
            } else if (this.g == 3) {
                if (n2 == 0) {
                    return 2;
                }
                if (n2 == 1) {
                    return 3;
                }
                if (n2 == 2) {
                    return 1;
                }
                if (n2 == 3) {
                    return 0;
                }
            }
        } else if (aji2 == ajn.ap) {
            if (this.g == 0) {
                if (n2 == 2) {
                    return 3;
                }
                if (n2 == 3) {
                    return 2;
                }
            } else if (this.g == 1) {
                if (n2 == 2) {
                    return 4;
                }
                if (n2 == 3) {
                    return 5;
                }
                if (n2 == 4) {
                    return 2;
                }
                if (n2 == 5) {
                    return 3;
                }
            } else if (this.g == 3) {
                if (n2 == 2) {
                    return 5;
                }
                if (n2 == 3) {
                    return 4;
                }
                if (n2 == 4) {
                    return 2;
                }
                if (n2 == 5) {
                    return 3;
                }
            }
        } else if (aji2 == ajn.aB) {
            if (this.g == 0) {
                if (n2 == 3) {
                    return 4;
                }
                if (n2 == 4) {
                    return 3;
                }
            } else if (this.g == 1) {
                if (n2 == 3) {
                    return 1;
                }
                if (n2 == 4) {
                    return 2;
                }
                if (n2 == 2) {
                    return 3;
                }
                if (n2 == 1) {
                    return 4;
                }
            } else if (this.g == 3) {
                if (n2 == 3) {
                    return 2;
                }
                if (n2 == 4) {
                    return 1;
                }
                if (n2 == 2) {
                    return 3;
                }
                if (n2 == 1) {
                    return 4;
                }
            }
        } else if (aji2 == ajn.bC || aji2 instanceof akk) {
            if (this.g == 0) {
                if (n2 == 0 || n2 == 2) {
                    return p.f[n2];
                }
            } else if (this.g == 1) {
                if (n2 == 2) {
                    return 1;
                }
                if (n2 == 0) {
                    return 3;
                }
                if (n2 == 1) {
                    return 2;
                }
                if (n2 == 3) {
                    return 0;
                }
            } else if (this.g == 3) {
                if (n2 == 2) {
                    return 3;
                }
                if (n2 == 0) {
                    return 1;
                }
                if (n2 == 1) {
                    return 2;
                }
                if (n2 == 3) {
                    return 0;
                }
            }
        } else if (aji2 == ajn.J || aji2 == ajn.F || aji2 == ajn.at || aji2 == ajn.z) {
            if (this.g == 0) {
                if (n2 == 2 || n2 == 3) {
                    return q.a[n2];
                }
            } else if (this.g == 1) {
                if (n2 == 2) {
                    return 4;
                }
                if (n2 == 3) {
                    return 5;
                }
                if (n2 == 4) {
                    return 2;
                }
                if (n2 == 5) {
                    return 3;
                }
            } else if (this.g == 3) {
                if (n2 == 2) {
                    return 5;
                }
                if (n2 == 3) {
                    return 4;
                }
                if (n2 == 4) {
                    return 2;
                }
                if (n2 == 5) {
                    return 3;
                }
            }
        }
        return n2;
    }

    protected void a(ahb ahb2, aji aji2, int n2, int n3, int n4, int n5, asv asv2) {
        int n6;
        int n7;
        int n8 = this.a(n3, n5);
        if (!asv2.b(n8, n7 = this.a(n4), n6 = this.b(n3, n5))) {
            return;
        }
        ahb2.d(n8, n7, n6, aji2, n2, 2);
    }

    protected aji a(ahb ahb2, int n2, int n3, int n4, asv asv2) {
        int n5;
        int n6;
        int n7 = this.a(n2, n4);
        if (!asv2.b(n7, n6 = this.a(n3), n5 = this.b(n2, n4))) {
            return ajn.a;
        }
        return ahb2.a(n7, n6, n5);
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7) {
        for (int i2 = n3; i2 <= n6; ++i2) {
            for (int i3 = n2; i3 <= n5; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    this.a(ahb2, ajn.a, 0, i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7, aji aji2, aji aji3, boolean bl2) {
        for (int i2 = n3; i2 <= n6; ++i2) {
            for (int i3 = n2; i3 <= n5; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    if (bl2 && this.a(ahb2, i3, i2, i4, asv2).o() == awt.a) continue;
                    if (i2 == n3 || i2 == n6 || i3 == n2 || i3 == n5 || i4 == n4 || i4 == n7) {
                        this.a(ahb2, aji2, 0, i3, i2, i4, asv2);
                        continue;
                    }
                    this.a(ahb2, aji3, 0, i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7, aji aji2, int n8, aji aji3, int n9, boolean bl2) {
        for (int i2 = n3; i2 <= n6; ++i2) {
            for (int i3 = n2; i3 <= n5; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    if (bl2 && this.a(ahb2, i3, i2, i4, asv2).o() == awt.a) continue;
                    if (i2 == n3 || i2 == n6 || i3 == n2 || i3 == n5 || i4 == n4 || i4 == n7) {
                        this.a(ahb2, aji2, n8, i3, i2, i4, asv2);
                        continue;
                    }
                    this.a(ahb2, aji3, n9, i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7, boolean bl2, Random random, avl avl2) {
        for (int i2 = n3; i2 <= n6; ++i2) {
            for (int i3 = n2; i3 <= n5; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    if (bl2 && this.a(ahb2, i3, i2, i4, asv2).o() == awt.a) continue;
                    avl2.a(random, i3, i2, i4, i2 == n3 || i2 == n6 || i3 == n2 || i3 == n5 || i4 == n4 || i4 == n7);
                    this.a(ahb2, avl2.a(), avl2.b(), i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void a(ahb ahb2, asv asv2, Random random, float f2, int n2, int n3, int n4, int n5, int n6, int n7, aji aji2, aji aji3, boolean bl2) {
        for (int i2 = n3; i2 <= n6; ++i2) {
            for (int i3 = n2; i3 <= n5; ++i3) {
                for (int i4 = n4; i4 <= n7; ++i4) {
                    if (random.nextFloat() > f2 || bl2 && this.a(ahb2, i3, i2, i4, asv2).o() == awt.a) continue;
                    if (i2 == n3 || i2 == n6 || i3 == n2 || i3 == n5 || i4 == n4 || i4 == n7) {
                        this.a(ahb2, aji2, 0, i3, i2, i4, asv2);
                        continue;
                    }
                    this.a(ahb2, aji3, 0, i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void a(ahb ahb2, asv asv2, Random random, float f2, int n2, int n3, int n4, aji aji2, int n5) {
        if (random.nextFloat() < f2) {
            this.a(ahb2, aji2, n5, n2, n3, n4, asv2);
        }
    }

    protected void a(ahb ahb2, asv asv2, int n2, int n3, int n4, int n5, int n6, int n7, aji aji2, boolean bl2) {
        float f2 = n5 - n2 + 1;
        float f3 = n6 - n3 + 1;
        float f4 = n7 - n4 + 1;
        float f5 = (float)n2 + f2 / 2.0f;
        float f6 = (float)n4 + f4 / 2.0f;
        for (int i2 = n3; i2 <= n6; ++i2) {
            float f7 = (float)(i2 - n3) / f3;
            for (int i3 = n2; i3 <= n5; ++i3) {
                float f8 = ((float)i3 - f5) / (f2 * 0.5f);
                for (int i4 = n4; i4 <= n7; ++i4) {
                    float f9;
                    float f10 = ((float)i4 - f6) / (f4 * 0.5f);
                    if (bl2 && this.a(ahb2, i3, i2, i4, asv2).o() == awt.a || !((f9 = f8 * f8 + f7 * f7 + f10 * f10) <= 1.05f)) continue;
                    this.a(ahb2, aji2, 0, i3, i2, i4, asv2);
                }
            }
        }
    }

    protected void b(ahb ahb2, int n2, int n3, int n4, asv asv2) {
        int n5;
        int n6;
        int n7 = this.a(n2, n4);
        if (!asv2.b(n7, n6 = this.a(n3), n5 = this.b(n2, n4))) {
            return;
        }
        while (!ahb2.c(n7, n6, n5) && n6 < 255) {
            ahb2.d(n7, n6, n5, ajn.a, 0, 2);
            ++n6;
        }
    }

    protected void b(ahb ahb2, aji aji2, int n2, int n3, int n4, int n5, asv asv2) {
        int n6;
        int n7;
        int n8 = this.a(n3, n5);
        if (!asv2.b(n8, n7 = this.a(n4), n6 = this.b(n3, n5))) {
            return;
        }
        while ((ahb2.c(n8, n7, n6) || ahb2.a(n8, n7, n6).o().d()) && n7 > 1) {
            ahb2.d(n8, n7, n6, aji2, n2, 2);
            --n7;
        }
    }

    protected boolean a(ahb ahb2, asv asv2, Random random, int n2, int n3, int n4, qx[] qxArray, int n5) {
        int n6;
        int n7;
        int n8 = this.a(n2, n4);
        if (asv2.b(n8, n7 = this.a(n3), n6 = this.b(n2, n4)) && ahb2.a(n8, n7, n6) != ajn.ae) {
            ahb2.d(n8, n7, n6, ajn.ae, 0, 2);
            aow aow2 = (aow)ahb2.o(n8, n7, n6);
            if (aow2 != null) {
                qx.a(random, qxArray, aow2, n5);
            }
            return true;
        }
        return false;
    }

    protected boolean a(ahb ahb2, asv asv2, Random random, int n2, int n3, int n4, int n5, qx[] qxArray, int n6) {
        int n7;
        int n8;
        int n9 = this.a(n2, n4);
        if (asv2.b(n9, n8 = this.a(n3), n7 = this.b(n2, n4)) && ahb2.a(n9, n8, n7) != ajn.z) {
            ahb2.d(n9, n8, n7, ajn.z, this.a(ajn.z, n5), 2);
            apb apb2 = (apb)ahb2.o(n9, n8, n7);
            if (apb2 != null) {
                qx.a(random, qxArray, apb2, n6);
            }
            return true;
        }
        return false;
    }

    protected void a(ahb ahb2, asv asv2, Random random, int n2, int n3, int n4, int n5) {
        int n6;
        int n7;
        int n8 = this.a(n2, n4);
        if (asv2.b(n8, n7 = this.a(n3), n6 = this.b(n2, n4))) {
            ach.a(ahb2, n8, n7, n6, n5, ajn.ao);
        }
    }
}

