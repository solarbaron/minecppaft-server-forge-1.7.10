/*
 * Decompiled with CFR 0.152.
 */
public class ji
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;

    @Override
    public void a(et et2) {
        this.e = et2.readUnsignedByte();
        this.a = et2.readInt();
        this.b = et2.readUnsignedByte();
        this.c = et2.readInt();
        this.d = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.e);
        et2.writeInt(this.a);
        et2.writeByte(this.b);
        et2.writeInt(this.c);
        et2.writeByte(this.d);
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

    public int g() {
        return this.e;
    }
}

