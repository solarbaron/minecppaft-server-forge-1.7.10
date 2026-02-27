/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class ans
extends aji {
    private static final int[][] a = new int[][]{{2, 6}, {3, 7}, {2, 3}, {6, 7}, {0, 4}, {1, 5}, {0, 1}, {4, 5}};
    private final aji b;
    private final int M;
    private boolean N;
    private int O;

    protected ans(aji aji2, int n2) {
        super(aji2.J);
        this.b = aji2;
        this.M = n2;
        this.c(aji2.v);
        this.b(aji2.w / 3.0f);
        this.a(aji2.H);
        this.g(255);
        this.a(abt.b);
    }

    @Override
    public void a(ahl ahl2, int n2, int n3, int n4) {
        if (this.N) {
            this.a(0.5f * (float)(this.O % 2), 0.5f * (float)(this.O / 2 % 2), 0.5f * (float)(this.O / 4 % 2), 0.5f + 0.5f * (float)(this.O % 2), 0.5f + 0.5f * (float)(this.O / 2 % 2), 0.5f + 0.5f * (float)(this.O / 4 % 2));
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
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
        return 10;
    }

    public void e(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        if ((n5 & 4) != 0) {
            this.a(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
        } else {
            this.a(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
        }
    }

    public static boolean a(aji aji2) {
        return aji2 instanceof ans;
    }

    private boolean f(ahl ahl2, int n2, int n3, int n4, int n5) {
        aji aji2 = ahl2.a(n2, n3, n4);
        return ans.a(aji2) && ahl2.e(n2, n3, n4) == n5;
    }

    public boolean f(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        int n6 = n5 & 3;
        float f2 = 0.5f;
        float f3 = 1.0f;
        if ((n5 & 4) != 0) {
            f2 = 0.0f;
            f3 = 0.5f;
        }
        float f4 = 0.0f;
        float f5 = 1.0f;
        float f6 = 0.0f;
        float f7 = 0.5f;
        boolean bl2 = true;
        if (n6 == 0) {
            f4 = 0.5f;
            f7 = 1.0f;
            aji aji2 = ahl2.a(n2 + 1, n3, n4);
            int n7 = ahl2.e(n2 + 1, n3, n4);
            if (ans.a(aji2) && (n5 & 4) == (n7 & 4)) {
                int n8 = n7 & 3;
                if (n8 == 3 && !this.f(ahl2, n2, n3, n4 + 1, n5)) {
                    f7 = 0.5f;
                    bl2 = false;
                } else if (n8 == 2 && !this.f(ahl2, n2, n3, n4 - 1, n5)) {
                    f6 = 0.5f;
                    bl2 = false;
                }
            }
        } else if (n6 == 1) {
            f5 = 0.5f;
            f7 = 1.0f;
            aji aji3 = ahl2.a(n2 - 1, n3, n4);
            int n9 = ahl2.e(n2 - 1, n3, n4);
            if (ans.a(aji3) && (n5 & 4) == (n9 & 4)) {
                int n10 = n9 & 3;
                if (n10 == 3 && !this.f(ahl2, n2, n3, n4 + 1, n5)) {
                    f7 = 0.5f;
                    bl2 = false;
                } else if (n10 == 2 && !this.f(ahl2, n2, n3, n4 - 1, n5)) {
                    f6 = 0.5f;
                    bl2 = false;
                }
            }
        } else if (n6 == 2) {
            f6 = 0.5f;
            f7 = 1.0f;
            aji aji4 = ahl2.a(n2, n3, n4 + 1);
            int n11 = ahl2.e(n2, n3, n4 + 1);
            if (ans.a(aji4) && (n5 & 4) == (n11 & 4)) {
                int n12 = n11 & 3;
                if (n12 == 1 && !this.f(ahl2, n2 + 1, n3, n4, n5)) {
                    f5 = 0.5f;
                    bl2 = false;
                } else if (n12 == 0 && !this.f(ahl2, n2 - 1, n3, n4, n5)) {
                    f4 = 0.5f;
                    bl2 = false;
                }
            }
        } else if (n6 == 3) {
            aji aji5 = ahl2.a(n2, n3, n4 - 1);
            int n13 = ahl2.e(n2, n3, n4 - 1);
            if (ans.a(aji5) && (n5 & 4) == (n13 & 4)) {
                int n14 = n13 & 3;
                if (n14 == 1 && !this.f(ahl2, n2 + 1, n3, n4, n5)) {
                    f5 = 0.5f;
                    bl2 = false;
                } else if (n14 == 0 && !this.f(ahl2, n2 - 1, n3, n4, n5)) {
                    f4 = 0.5f;
                    bl2 = false;
                }
            }
        }
        this.a(f4, f2, f6, f5, f3, f7);
        return bl2;
    }

    public boolean g(ahl ahl2, int n2, int n3, int n4) {
        int n5 = ahl2.e(n2, n3, n4);
        int n6 = n5 & 3;
        float f2 = 0.5f;
        float f3 = 1.0f;
        if ((n5 & 4) != 0) {
            f2 = 0.0f;
            f3 = 0.5f;
        }
        float f4 = 0.0f;
        float f5 = 0.5f;
        float f6 = 0.5f;
        float f7 = 1.0f;
        boolean bl2 = false;
        if (n6 == 0) {
            aji aji2 = ahl2.a(n2 - 1, n3, n4);
            int n7 = ahl2.e(n2 - 1, n3, n4);
            if (ans.a(aji2) && (n5 & 4) == (n7 & 4)) {
                int n8 = n7 & 3;
                if (n8 == 3 && !this.f(ahl2, n2, n3, n4 - 1, n5)) {
                    f6 = 0.0f;
                    f7 = 0.5f;
                    bl2 = true;
                } else if (n8 == 2 && !this.f(ahl2, n2, n3, n4 + 1, n5)) {
                    f6 = 0.5f;
                    f7 = 1.0f;
                    bl2 = true;
                }
            }
        } else if (n6 == 1) {
            aji aji3 = ahl2.a(n2 + 1, n3, n4);
            int n9 = ahl2.e(n2 + 1, n3, n4);
            if (ans.a(aji3) && (n5 & 4) == (n9 & 4)) {
                f4 = 0.5f;
                f5 = 1.0f;
                int n10 = n9 & 3;
                if (n10 == 3 && !this.f(ahl2, n2, n3, n4 - 1, n5)) {
                    f6 = 0.0f;
                    f7 = 0.5f;
                    bl2 = true;
                } else if (n10 == 2 && !this.f(ahl2, n2, n3, n4 + 1, n5)) {
                    f6 = 0.5f;
                    f7 = 1.0f;
                    bl2 = true;
                }
            }
        } else if (n6 == 2) {
            aji aji4 = ahl2.a(n2, n3, n4 - 1);
            int n11 = ahl2.e(n2, n3, n4 - 1);
            if (ans.a(aji4) && (n5 & 4) == (n11 & 4)) {
                f6 = 0.0f;
                f7 = 0.5f;
                int n12 = n11 & 3;
                if (n12 == 1 && !this.f(ahl2, n2 - 1, n3, n4, n5)) {
                    bl2 = true;
                } else if (n12 == 0 && !this.f(ahl2, n2 + 1, n3, n4, n5)) {
                    f4 = 0.5f;
                    f5 = 1.0f;
                    bl2 = true;
                }
            }
        } else if (n6 == 3) {
            aji aji5 = ahl2.a(n2, n3, n4 + 1);
            int n13 = ahl2.e(n2, n3, n4 + 1);
            if (ans.a(aji5) && (n5 & 4) == (n13 & 4)) {
                int n14 = n13 & 3;
                if (n14 == 1 && !this.f(ahl2, n2 - 1, n3, n4, n5)) {
                    bl2 = true;
                } else if (n14 == 0 && !this.f(ahl2, n2 + 1, n3, n4, n5)) {
                    f4 = 0.5f;
                    f5 = 1.0f;
                    bl2 = true;
                }
            }
        }
        if (bl2) {
            this.a(f4, f2, f6, f5, f3, f7);
        }
        return bl2;
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        this.e(ahb2, n2, n3, n4);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        boolean bl2 = this.f((ahl)ahb2, n2, n3, n4);
        super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        if (bl2 && this.g(ahb2, n2, n3, n4)) {
            super.a(ahb2, n2, n3, n4, azt2, list, sa2);
        }
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
        this.b.a(ahb2, n2, n3, n4, yz2);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, int n5) {
        this.b.b(ahb2, n2, n3, n4, n5);
    }

    @Override
    public float a(sa sa2) {
        return this.b.a(sa2);
    }

    @Override
    public int a(ahb ahb2) {
        return this.b.a(ahb2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2, azw azw2) {
        this.b.a(ahb2, n2, n3, n4, sa2, azw2);
    }

    @Override
    public boolean v() {
        return this.b.v();
    }

    @Override
    public boolean a(int n2, boolean bl2) {
        return this.b.a(n2, bl2);
    }

    @Override
    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return this.b.c(ahb2, n2, n3, n4);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4) {
        this.a(ahb2, n2, n3, n4, ajn.a);
        this.b.b(ahb2, n2, n3, n4);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
        this.b.a(ahb2, n2, n3, n4, aji2, n5);
    }

    @Override
    public void b(ahb ahb2, int n2, int n3, int n4, sa sa2) {
        this.b.b(ahb2, n2, n3, n4, sa2);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
        this.b.a(ahb2, n2, n3, n4, random);
    }

    @Override
    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        return this.b.a(ahb2, n2, n3, n4, yz2, 0, 0.0f, 0.0f, 0.0f);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, agw agw2) {
        this.b.a(ahb2, n2, n3, n4, agw2);
    }

    @Override
    public awv f(int n2) {
        return this.b.f(this.M);
    }

    @Override
    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
        int n5 = qh.c((double)(sv2.y * 4.0f / 360.0f) + 0.5) & 3;
        int n6 = ahb2.e(n2, n3, n4) & 4;
        if (n5 == 0) {
            ahb2.a(n2, n3, n4, 2 | n6, 2);
        }
        if (n5 == 1) {
            ahb2.a(n2, n3, n4, 1 | n6, 2);
        }
        if (n5 == 2) {
            ahb2.a(n2, n3, n4, 3 | n6, 2);
        }
        if (n5 == 3) {
            ahb2.a(n2, n3, n4, 0 | n6, 2);
        }
    }

    @Override
    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        if (n5 == 0 || n5 != 1 && (double)f3 > 0.5) {
            return n6 | 4;
        }
        return n6;
    }

    /*
     * WARNING - void declaration
     */
    @Override
    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        void var12_16;
        void object;
        azu[] azuArray = new azu[8];
        int n5 = ahb2.e(n2, n3, n4);
        int n6 = n5 & 3;
        boolean bl2 = (n5 & 4) == 4;
        int[] nArray = a[n6 + (bl2 ? 4 : 0)];
        this.N = true;
        boolean i2 = false;
        while (object < 8) {
            this.O = object;
            int[] nArray2 = nArray;
            int n7 = nArray2.length;
            for (int i3 = 0; i3 < n7; ++i3) {
                int n8 = nArray2[i3];
                if (n8 != object) continue;
            }
            azuArray[object] = super.a(ahb2, n2, n3, n4, azw2, azw3);
            ++object;
        }
        for (int i3 : nArray) {
            azuArray[i3] = null;
        }
        Object var12_15 = null;
        double d2 = 0.0;
        for (azu azu2 : azuArray) {
            double d3;
            if (azu2 == null || !((d3 = azu2.f.e(azw3)) > d2)) continue;
            azu azu3 = azu2;
            d2 = d3;
        }
        return var12_16;
    }
}

