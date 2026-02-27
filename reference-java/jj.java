/*
 * Decompiled with CFR 0.152.
 */
public class jj
extends ft {
    private int a;
    private int b;
    private int c;

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readByte();
        this.c = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
        et2.writeInt(this.c);
    }

    public void a(io io2) {
        io2.a(this);
    }

    public int d() {
        return this.b;
    }

    public int e() {
        return this.c;
    }
}

