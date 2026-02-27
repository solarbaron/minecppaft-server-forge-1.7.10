/*
 * Decompiled with CFR 0.152.
 */
public class gq
extends ft {
    private int a;
    private int b;
    private add c;

    public gq() {
    }

    public gq(int n2, int n3, add add2) {
        this.a = n2;
        this.b = n3;
        this.c = add2 == null ? null : add2.m();
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
        this.b = et2.readShort();
        this.c = et2.c();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b);
        et2.a(this.c);
    }
}

