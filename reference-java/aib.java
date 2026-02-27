/*
 * Decompiled with CFR 0.152.
 */
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class aib {
    private axn c;
    private axn d;
    private ahy e = new ahy(this);
    private List f = new ArrayList();

    protected aib() {
        this.f.add(ahu.s);
        this.f.add(ahu.p);
        this.f.add(ahu.t);
        this.f.add(ahu.H);
        this.f.add(ahu.G);
        this.f.add(ahu.J);
        this.f.add(ahu.K);
    }

    public aib(long l2, ahm ahm2) {
        this();
        axn[] axnArray = axn.a(l2, ahm2);
        this.c = axnArray[0];
        this.d = axnArray[1];
    }

    public aib(ahb ahb2) {
        this(ahb2.H(), ahb2.N().u());
    }

    public List a() {
        return this.f;
    }

    public ahu a(int n2, int n3) {
        return this.e.b(n2, n3);
    }

    public float[] a(float[] fArray, int n2, int n3, int n4, int n5) {
        axl.a();
        if (fArray == null || fArray.length < n4 * n5) {
            fArray = new float[n4 * n5];
        }
        int[] nArray = this.d.a(n2, n3, n4, n5);
        for (int i2 = 0; i2 < n4 * n5; ++i2) {
            try {
                float f2 = (float)ahu.d(nArray[i2]).h() / 65536.0f;
                if (f2 > 1.0f) {
                    f2 = 1.0f;
                }
                fArray[i2] = f2;
                continue;
            }
            catch (Throwable throwable) {
                b b2 = b.a(throwable, "Invalid Biome id");
                k k2 = b2.a("DownfallBlock");
                k2.a("biome id", i2);
                k2.a("downfalls[] size", fArray.length);
                k2.a("x", n2);
                k2.a("z", n3);
                k2.a("w", n4);
                k2.a("h", n5);
                throw new s(b2);
            }
        }
        return fArray;
    }

    public ahu[] a(ahu[] ahuArray, int n2, int n3, int n4, int n5) {
        axl.a();
        if (ahuArray == null || ahuArray.length < n4 * n5) {
            ahuArray = new ahu[n4 * n5];
        }
        int[] nArray = this.c.a(n2, n3, n4, n5);
        try {
            for (int i2 = 0; i2 < n4 * n5; ++i2) {
                ahuArray[i2] = ahu.d(nArray[i2]);
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Invalid Biome id");
            k k2 = b2.a("RawBiomeBlock");
            k2.a("biomes[] size", ahuArray.length);
            k2.a("x", n2);
            k2.a("z", n3);
            k2.a("w", n4);
            k2.a("h", n5);
            throw new s(b2);
        }
        return ahuArray;
    }

    public ahu[] b(ahu[] ahuArray, int n2, int n3, int n4, int n5) {
        return this.a(ahuArray, n2, n3, n4, n5, true);
    }

    public ahu[] a(ahu[] ahuArray, int n2, int n3, int n4, int n5, boolean bl2) {
        axl.a();
        if (ahuArray == null || ahuArray.length < n4 * n5) {
            ahuArray = new ahu[n4 * n5];
        }
        if (bl2 && n4 == 16 && n5 == 16 && (n2 & 0xF) == 0 && (n3 & 0xF) == 0) {
            ahu[] ahuArray2 = this.e.d(n2, n3);
            System.arraycopy(ahuArray2, 0, ahuArray, 0, n4 * n5);
            return ahuArray;
        }
        int[] nArray = this.d.a(n2, n3, n4, n5);
        for (int i2 = 0; i2 < n4 * n5; ++i2) {
            ahuArray[i2] = ahu.d(nArray[i2]);
        }
        return ahuArray;
    }

    public boolean a(int n2, int n3, int n4, List list) {
        axl.a();
        int n5 = n2 - n4 >> 2;
        int n6 = n3 - n4 >> 2;
        int n7 = n2 + n4 >> 2;
        int n8 = n3 + n4 >> 2;
        int n9 = n7 - n5 + 1;
        int n10 = n8 - n6 + 1;
        int[] nArray = this.c.a(n5, n6, n9, n10);
        try {
            for (int i2 = 0; i2 < n9 * n10; ++i2) {
                ahu ahu2 = ahu.d(nArray[i2]);
                if (list.contains(ahu2)) continue;
                return false;
            }
        }
        catch (Throwable throwable) {
            b b2 = b.a(throwable, "Invalid Biome id");
            k k2 = b2.a("Layer");
            k2.a("Layer", this.c.toString());
            k2.a("x", n2);
            k2.a("z", n3);
            k2.a("radius", n4);
            k2.a("allowed", list);
            throw new s(b2);
        }
        return true;
    }

    public agt a(int n2, int n3, int n4, List list, Random random) {
        axl.a();
        int n5 = n2 - n4 >> 2;
        int n6 = n3 - n4 >> 2;
        int n7 = n2 + n4 >> 2;
        int n8 = n3 + n4 >> 2;
        int n9 = n7 - n5 + 1;
        int n10 = n8 - n6 + 1;
        int[] nArray = this.c.a(n5, n6, n9, n10);
        agt agt2 = null;
        int n11 = 0;
        for (int i2 = 0; i2 < n9 * n10; ++i2) {
            int n12 = n5 + i2 % n9 << 2;
            int n13 = n6 + i2 / n9 << 2;
            ahu ahu2 = ahu.d(nArray[i2]);
            if (!list.contains(ahu2) || agt2 != null && random.nextInt(n11 + 1) != 0) continue;
            agt2 = new agt(n12, 0, n13);
            ++n11;
        }
        return agt2;
    }

    public void b() {
        this.e.a();
    }
}

