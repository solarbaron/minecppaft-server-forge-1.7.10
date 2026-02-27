/*
 * Decompiled with CFR 0.152.
 */
public class ge
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;

    public ge() {
    }

    public ge(int n2, int n3, int n4, int n5, int n6) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = n5;
        this.e = n6;
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeByte(this.e);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

