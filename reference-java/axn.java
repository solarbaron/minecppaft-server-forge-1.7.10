/*
 * Decompiled with CFR 0.152.
 */
public abstract class axn {
    private long c;
    protected axn a;
    private long d;
    protected long b;

    public static axn[] a(long l2, ahm ahm2) {
        boolean bl2 = false;
        axn axn2 = new axm(1L);
        axn2 = new axj(2000L, axn2);
        axn2 = new axb(1L, axn2);
        axn2 = new ayc(2001L, axn2);
        axn2 = new axb(2L, axn2);
        axn2 = new axb(50L, axn2);
        axn2 = new axb(70L, axn2);
        axn2 = new axs(2L, axn2);
        axn2 = new axd(2L, axn2);
        axn2 = new axb(3L, axn2);
        axn2 = new awy(2L, axn2, axa.a);
        axn2 = new awy(2L, axn2, axa.b);
        axn2 = new awy(3L, axn2, axa.c);
        axn2 = new ayc(2002L, axn2);
        axn2 = new ayc(2003L, axn2);
        axn2 = new axb(4L, axn2);
        axn2 = new axc(5L, axn2);
        axn2 = new awx(4L, axn2);
        axn2 = ayc.b(1000L, axn2, 0);
        int n2 = 4;
        if (ahm2 == ahm.d) {
            n2 = 6;
        }
        if (bl2) {
            n2 = 4;
        }
        axn axn3 = axn2;
        axn3 = ayc.b(1000L, axn3, 0);
        axn3 = new axt(100L, axn3);
        axn axn4 = axn2;
        axn4 = new axf(200L, axn4, ahm2);
        if (!bl2) {
            axn4 = ayc.b(1000L, axn4, 2);
            axn4 = new axe(1000L, axn4);
        }
        axn axn5 = axn3;
        axn5 = ayc.b(1000L, axn5, 2);
        axn4 = new axr(1000L, axn4, axn5);
        axn3 = ayc.b(1000L, axn3, 2);
        axn3 = ayc.b(1000L, axn3, n2);
        axn3 = new axu(1L, axn3);
        axn3 = new axx(1000L, axn3);
        axn4 = new axq(1001L, axn4);
        for (int i2 = 0; i2 < n2; ++i2) {
            axn4 = new ayc(1000 + i2, axn4);
            if (i2 == 0) {
                axn4 = new axb(3L, axn4);
            }
            if (i2 != 1) continue;
            axn4 = new axw(1000L, axn4);
        }
        axn4 = new axx(1000L, axn4);
        axn4 = new axv(100L, axn4, axn3);
        axn axn6 = axn4;
        ayb ayb2 = new ayb(10L, axn4);
        axn4.a(l2);
        ayb2.a(l2);
        return new axn[]{axn4, ayb2, axn6};
    }

    public axn(long l2) {
        this.b = l2;
        this.b *= this.b * 6364136223846793005L + 1442695040888963407L;
        this.b += l2;
        this.b *= this.b * 6364136223846793005L + 1442695040888963407L;
        this.b += l2;
        this.b *= this.b * 6364136223846793005L + 1442695040888963407L;
        this.b += l2;
    }

    public void a(long l2) {
        this.c = l2;
        if (this.a != null) {
            this.a.a(l2);
        }
        this.c *= this.c * 6364136223846793005L + 1442695040888963407L;
        this.c += this.b;
        this.c *= this.c * 6364136223846793005L + 1442695040888963407L;
        this.c += this.b;
        this.c *= this.c * 6364136223846793005L + 1442695040888963407L;
        this.c += this.b;
    }

    public void a(long l2, long l3) {
        this.d = this.c;
        this.d *= this.d * 6364136223846793005L + 1442695040888963407L;
        this.d += l2;
        this.d *= this.d * 6364136223846793005L + 1442695040888963407L;
        this.d += l3;
        this.d *= this.d * 6364136223846793005L + 1442695040888963407L;
        this.d += l2;
        this.d *= this.d * 6364136223846793005L + 1442695040888963407L;
        this.d += l3;
    }

    protected int a(int n2) {
        int n3 = (int)((this.d >> 24) % (long)n2);
        if (n3 < 0) {
            n3 += n2;
        }
        this.d *= this.d * 6364136223846793005L + 1442695040888963407L;
        this.d += this.c;
        return n3;
    }

    public abstract int[] a(int var1, int var2, int var3, int var4);

    protected static boolean a(int n2, int n3) {
        if (n2 == n3) {
            return true;
        }
        if (n2 == ahu.aa.ay || n2 == ahu.ab.ay) {
            return n3 == ahu.aa.ay || n3 == ahu.ab.ay;
        }
        try {
            if (ahu.d(n2) != null && ahu.d(n3) != null) {
                return ahu.d(n2).a(ahu.d(n3));
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Comparing biomes");
            k k2 = b2.a("Biomes being compared");
            k2.a("Biome A ID", n2);
            k2.a("Biome B ID", n3);
            k2.a("Biome A", new axo(n2));
            k2.a("Biome B", new axp(n3));
            throw new s(b2);
        }
        return false;
    }

    protected static boolean b(int n2) {
        return n2 == ahu.o.ay || n2 == ahu.M.ay || n2 == ahu.y.ay;
    }

    protected int a(int ... nArray) {
        return nArray[this.a(nArray.length)];
    }

    protected int b(int n2, int n3, int n4, int n5) {
        if (n3 == n4 && n4 == n5) {
            return n3;
        }
        if (n2 == n3 && n2 == n4) {
            return n2;
        }
        if (n2 == n3 && n2 == n5) {
            return n2;
        }
        if (n2 == n4 && n2 == n5) {
            return n2;
        }
        if (n2 == n3 && n4 != n5) {
            return n2;
        }
        if (n2 == n4 && n3 != n5) {
            return n2;
        }
        if (n2 == n5 && n3 != n4) {
            return n2;
        }
        if (n3 == n4 && n2 != n5) {
            return n3;
        }
        if (n3 == n5 && n2 != n4) {
            return n3;
        }
        if (n4 == n5 && n2 != n3) {
            return n4;
        }
        return this.a(new int[]{n2, n3, n4, n5});
    }
}

