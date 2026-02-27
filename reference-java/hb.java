/*
 * Decompiled with CFR 0.152.
 */
public class hb
extends ft {
    private String a;
    private float b;
    private float c;
    private float d;
    private float e;
    private float f;
    private float g;
    private float h;
    private int i;

    public hb() {
    }

    public hb(String string, float f2, float f3, float f4, float f5, float f6, float f7, float f8, int n2) {
        this.a = string;
        this.b = f2;
        this.c = f3;
        this.d = f4;
        this.e = f5;
        this.f = f6;
        this.g = f7;
        this.h = f8;
        this.i = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(64);
        this.b = et2.readFloat();
        this.c = et2.readFloat();
        this.d = et2.readFloat();
        this.e = et2.readFloat();
        this.f = et2.readFloat();
        this.g = et2.readFloat();
        this.h = et2.readFloat();
        this.i = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeFloat(this.b);
        et2.writeFloat(this.c);
        et2.writeFloat(this.d);
        et2.writeFloat(this.e);
        et2.writeFloat(this.f);
        et2.writeFloat(this.g);
        et2.writeFloat(this.h);
        et2.writeInt(this.i);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

