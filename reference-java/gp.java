/*
 * Decompiled with CFR 0.152.
 */
public class gp
extends ft {
    private int a;
    private int b;
    private int c;

    public gp() {
    }

    public gp(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readUnsignedByte();
        this.b = et2.readShort();
        this.c = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b);
        et2.writeShort(this.c);
    }
}

