/*
 * Decompiled with CFR 0.152.
 */
public class gl
extends ft {
    private int a;
    private short b;
    private boolean c;

    public gl() {
    }

    public gl(int n2, short s2, boolean bl2) {
        this.a = n2;
        this.b = s2;
        this.c = bl2;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readUnsignedByte();
        this.b = et2.readShort();
        this.c = et2.readBoolean();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b);
        et2.writeBoolean(this.c);
    }

    @Override
    public String b() {
        return String.format("id=%d, uid=%d, accepted=%b", this.a, this.b, this.c);
    }
}

