/*
 * Decompiled with CFR 0.152.
 */
public class he
extends ft {
    private int a;
    private byte[] b;

    public he() {
    }

    public he(int n2, byte[] byArray) {
        this.a = n2;
        this.b = byArray;
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.b = new byte[et2.readUnsignedShort()];
        et2.readBytes(this.b);
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.writeShort(this.b.length);
        et2.writeBytes(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, length=%d", this.a, this.b.length);
    }
}

