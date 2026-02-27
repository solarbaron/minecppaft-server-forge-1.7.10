/*
 * Decompiled with CFR 0.152.
 */
public class gg
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private aji f;

    public gg() {
    }

    public gg(int n2, int n3, int n4, aji aji2, int n5, int n6) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = n6;
        this.f = aji2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readShort();
        this.c = et2.readInt();
        this.d = et2.readUnsignedByte();
        this.e = et2.readUnsignedByte();
        this.f = aji.e(et2.a() & 0xFFF);
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeShort(this.b);
        et2.writeInt(this.c);
        et2.writeByte(this.d);
        et2.writeByte(this.e);
        et2.b(aji.b(this.f) & 0xFFF);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

