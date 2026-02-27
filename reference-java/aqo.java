/*
 * Decompiled with CFR 0.152.
 */
public abstract class aqo {
    public static final float[] a = new float[]{1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.25f, 0.5f, 0.75f};
    public ahb b;
    public ahm c;
    public String d;
    public aib e;
    public boolean f;
    public boolean g;
    public float[] h = new float[16];
    public int i;
    private float[] j = new float[4];

    public final void a(ahb ahb2) {
        this.b = ahb2;
        this.c = ahb2.N().u();
        this.d = ahb2.N().y();
        this.b();
        this.a();
    }

    protected void a() {
        float f2 = 0.0f;
        for (int i2 = 0; i2 <= 15; ++i2) {
            float f3 = 1.0f - (float)i2 / 15.0f;
            this.h[i2] = (1.0f - f3) / (f3 * 3.0f + 1.0f) * (1.0f - f2) + f2;
        }
    }

    protected void b() {
        if (this.b.N().u() == ahm.c) {
            ass ass2 = ass.a(this.b.N().y());
            this.e = new aie(ahu.d(ass2.a()), 0.5f);
        } else {
            this.e = new aib(this.b);
        }
    }

    public apu c() {
        if (this.c == ahm.c) {
            return new aqu(this.b, this.b.H(), this.b.N().s(), this.d);
        }
        return new aqz(this.b, this.b.H(), this.b.N().s());
    }

    public boolean a(int n2, int n3) {
        return this.b.b(n2, n3) == ajn.c;
    }

    public float a(long l2, float f2) {
        int n2 = (int)(l2 % 24000L);
        float f3 = ((float)n2 + f2) / 24000.0f - 0.25f;
        if (f3 < 0.0f) {
            f3 += 1.0f;
        }
        if (f3 > 1.0f) {
            f3 -= 1.0f;
        }
        float f4 = f3;
        f3 = 1.0f - (float)((Math.cos((double)f3 * Math.PI) + 1.0) / 2.0);
        f3 = f4 + (f3 - f4) / 3.0f;
        return f3;
    }

    public int a(long l2) {
        return (int)(l2 / 24000L % 8L + 8L) % 8;
    }

    public boolean d() {
        return true;
    }

    public boolean e() {
        return true;
    }

    public static aqo a(int n2) {
        if (n2 == -1) {
            return new aqp();
        }
        if (n2 == 0) {
            return new aqq();
        }
        if (n2 == 1) {
            return new aqr();
        }
        return null;
    }

    public r h() {
        return null;
    }

    public int i() {
        if (this.c == ahm.c) {
            return 4;
        }
        return 64;
    }

    public abstract String l();
}

