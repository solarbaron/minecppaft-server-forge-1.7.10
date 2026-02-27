/*
 * Decompiled with CFR 0.152.
 */
public class ia
extends ft {
    private float a;
    private int b;
    private int c;

    public ia() {
    }

    public ia(float f2, int n2, int n3) {
        this.a = f2;
        this.b = n2;
        this.c = n3;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readFloat();
        this.c = et2.readShort();
        this.b = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.writeFloat(this.a);
        et2.writeShort(this.c);
        et2.writeShort(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

