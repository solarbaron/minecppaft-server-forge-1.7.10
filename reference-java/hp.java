/*
 * Decompiled with CFR 0.152.
 */
public class hp
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;

    public hp() {
    }

    public hp(yz yz2, int n2, int n3, int n4) {
        this.b = n2;
        this.c = n3;
        this.d = n4;
        this.a = yz2.y();
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readInt();
        this.c = et2.readByte();
        this.d = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b);
        et2.writeByte(this.c);
        et2.writeInt(this.d);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

