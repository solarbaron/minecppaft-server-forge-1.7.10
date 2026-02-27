/*
 * Decompiled with CFR 0.152.
 */
public class ip
extends ft {
    private int a;
    private int b;

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
    }

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%d", this.a, this.b);
    }

    public int d() {
        return this.b;
    }
}

