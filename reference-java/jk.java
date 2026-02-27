/*
 * Decompiled with CFR 0.152.
 */
public class jk
extends ft {
    private float a;
    private float b;
    private boolean c;
    private boolean d;

    @Override
    public void a(et et2) {
        this.a = et2.readFloat();
        this.b = et2.readFloat();
        this.c = et2.readBoolean();
        this.d = et2.readBoolean();
    }

    @Override
    public void b(et et2) {
        et2.writeFloat(this.a);
        et2.writeFloat(this.b);
        et2.writeBoolean(this.c);
        et2.writeBoolean(this.d);
    }

    public void a(io io2) {
        io2.a(this);
    }

    public float c() {
        return this.a;
    }

    public float d() {
        return this.b;
    }

    public boolean e() {
        return this.c;
    }

    public boolean f() {
        return this.d;
    }
}

