/*
 * Decompiled with CFR 0.152.
 */
public class ahv {
    public float a;
    public float b;

    public ahv(float f2, float f3) {
        this.a = f2;
        this.b = f3;
    }

    public ahv a() {
        return new ahv(this.a * 0.8f, this.b * 0.6f);
    }
}

