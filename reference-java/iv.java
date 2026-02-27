/*
 * Decompiled with CFR 0.152.
 */
public class iv
extends ft {
    private int a;
    private short b;
    private boolean c;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
        this.b = et2.readShort();
        this.c = et2.readByte() != 0;
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b);
        et2.writeByte(this.c ? 1 : 0);
    }

    @Override
    public String b() {
        return String.format("id=%d, uid=%d, accepted=%b", this.a, this.b, this.c);
    }

    public int c() {
        return this.a;
    }

    public short d() {
        return this.b;
    }
}

