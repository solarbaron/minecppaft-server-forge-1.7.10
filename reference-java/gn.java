/*
 * Decompiled with CFR 0.152.
 */
public class gn
extends ft {
    private int a;
    private int b;
    private String c;
    private int d;
    private boolean e;
    private int f;

    public gn() {
    }

    public gn(int n2, int n3, String string, int n4, boolean bl2) {
        this.a = n2;
        this.b = n3;
        this.c = string;
        this.d = n4;
        this.e = bl2;
    }

    public gn(int n2, int n3, String string, int n4, boolean bl2, int n5) {
        this(n2, n3, string, n4, bl2);
        this.f = n5;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readUnsignedByte();
        this.b = et2.readUnsignedByte();
        this.c = et2.c(32);
        this.d = et2.readUnsignedByte();
        this.e = et2.readBoolean();
        if (this.b == 11) {
            this.f = et2.readInt();
        }
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeByte(this.b);
        et2.a(this.c);
        et2.writeByte(this.d);
        et2.writeBoolean(this.e);
        if (this.b == 11) {
            et2.writeInt(this.f);
        }
    }
}

