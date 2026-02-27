/*
 * Decompiled with CFR 0.152.
 */
public class ib
extends ft {
    private float a;
    private int b;
    private float c;

    public ib() {
    }

    public ib(float f2, int n2, float f3) {
        this.a = f2;
        this.b = n2;
        this.c = f3;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readFloat();
        this.b = et2.readShort();
        this.c = et2.readFloat();
    }

    @Override
    public void b(et et2) {
        et2.writeFloat(this.a);
        et2.writeShort(this.b);
        et2.writeFloat(this.c);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

