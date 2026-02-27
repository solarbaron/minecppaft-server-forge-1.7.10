/*
 * Decompiled with CFR 0.152.
 */
public class fu
extends ft {
    private double a;
    private double b;
    private double c;
    private float d;
    private float e;
    private boolean f;

    public fu() {
    }

    public fu(double d2, double d3, double d4, float f2, float f3, boolean bl2) {
        this.a = d2;
        this.b = d3;
        this.c = d4;
        this.d = f2;
        this.e = f3;
        this.f = bl2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readDouble();
        this.b = et2.readDouble();
        this.c = et2.readDouble();
        this.d = et2.readFloat();
        this.e = et2.readFloat();
        this.f = et2.readBoolean();
    }

    @Override
    public void b(et et2) {
        et2.writeDouble(this.a);
        et2.writeDouble(this.b);
        et2.writeDouble(this.c);
        et2.writeFloat(this.d);
        et2.writeFloat(this.e);
        et2.writeBoolean(this.f);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

