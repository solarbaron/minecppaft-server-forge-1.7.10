/*
 * Decompiled with CFR 0.152.
 */
public class in
extends ft {
    private int a;
    private byte b;
    private byte c;
    private short d;

    public in() {
    }

    public in(int n2, rw rw2) {
        this.a = n2;
        this.b = (byte)(rw2.a() & 0xFF);
        this.c = (byte)(rw2.c() & 0xFF);
        this.d = rw2.b() > Short.MAX_VALUE ? (short)Short.MAX_VALUE : (short)rw2.b();
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readByte();
        this.c = et2.readByte();
        this.d = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
        et2.writeByte(this.c);
        et2.writeShort(this.d);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

