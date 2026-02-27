/*
 * Decompiled with CFR 0.152.
 */
public class gh
extends ft {
    private int a;
    private int b;
    private int c;
    private aji d;
    private int e;

    public gh() {
    }

    public gh(int n2, int n3, int n4, ahb ahb2) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
        this.d = ahb2.a(n2, n3, n4);
        this.e = ahb2.e(n2, n3, n4);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readUnsignedByte();
        this.c = et2.readInt();
        this.d = aji.e(et2.a());
        this.e = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
        et2.writeInt(this.c);
        et2.b(aji.b(this.d));
        et2.writeByte(this.e);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("type=%d, data=%d, x=%d, y=%d, z=%d", aji.b(this.d), this.e, this.a, this.b, this.c);
    }
}

