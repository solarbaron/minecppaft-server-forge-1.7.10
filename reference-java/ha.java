/*
 * Decompiled with CFR 0.152.
 */
public class ha
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private boolean f;

    public ha() {
    }

    public ha(int n2, int n3, int n4, int n5, int n6, boolean bl2) {
        this.a = n2;
        this.c = n3;
        this.d = n4;
        this.e = n5;
        this.b = n6;
        this.f = bl2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readByte() & 0xFF;
        this.e = et2.readInt();
        this.b = et2.readInt();
        this.f = et2.readBoolean();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.c);
        et2.writeByte(this.d & 0xFF);
        et2.writeInt(this.e);
        et2.writeInt(this.b);
        et2.writeBoolean(this.f);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

