/*
 * Decompiled with CFR 0.152.
 */
public class jo
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private add e;
    private float f;
    private float g;
    private float h;

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readUnsignedByte();
        this.c = et2.readInt();
        this.d = et2.readUnsignedByte();
        this.e = et2.c();
        this.f = (float)et2.readUnsignedByte() / 16.0f;
        this.g = (float)et2.readUnsignedByte() / 16.0f;
        this.h = (float)et2.readUnsignedByte() / 16.0f;
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
        et2.writeInt(this.c);
        et2.writeByte(this.d);
        et2.a(this.e);
        et2.writeByte((int)(this.f * 16.0f));
        et2.writeByte((int)(this.g * 16.0f));
        et2.writeByte((int)(this.h * 16.0f));
    }

    public void a(io io2) {
        io2.a(this);
    }

    public int c() {
        return this.a;
    }

    public int d() {
        return this.b;
    }

    public int e() {
        return this.c;
    }

    public int f() {
        return this.d;
    }

    public add g() {
        return this.e;
    }

    public float h() {
        return this.f;
    }

    public float i() {
        return this.g;
    }

    public float j() {
        return this.h;
    }
}

