/*
 * Decompiled with CFR 0.152.
 */
public class iw
extends ft {
    private int a;
    private int b;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
        this.b = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeByte(this.b);
    }

    @Override
    public String b() {
        return String.format("id=%d, button=%d", this.a, this.b);
    }

    public int c() {
        return this.a;
    }

    public int d() {
        return this.b;
    }
}

