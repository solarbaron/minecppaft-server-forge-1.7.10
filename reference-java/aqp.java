/*
 * Decompiled with CFR 0.152.
 */
public class aqp
extends aqo {
    @Override
    public void b() {
        this.e = new aie(ahu.w, 0.0f);
        this.f = true;
        this.g = true;
        this.i = -1;
    }

    @Override
    protected void a() {
        float f2 = 0.1f;
        for (int i2 = 0; i2 <= 15; ++i2) {
            float f3 = 1.0f - (float)i2 / 15.0f;
            this.h[i2] = (1.0f - f3) / (f3 * 3.0f + 1.0f) * (1.0f - f2) + f2;
        }
    }

    @Override
    public apu c() {
        return new aqv(this.b, this.b.H());
    }

    @Override
    public boolean d() {
        return false;
    }

    @Override
    public boolean a(int n2, int n3) {
        return false;
    }

    @Override
    public float a(long l2, float f2) {
        return 0.5f;
    }

    @Override
    public boolean e() {
        return false;
    }

    @Override
    public String l() {
        return "Nether";
    }
}

