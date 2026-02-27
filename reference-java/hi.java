/*
 * Decompiled with CFR 0.152.
 */
public class hi
extends hf {
    public hi() {
        this.g = true;
    }

    public hi(int n2, byte by2, byte by3) {
        super(n2);
        this.e = by2;
        this.f = by3;
        this.g = true;
    }

    @Override
    public void a(et et2) {
        super.a(et2);
        this.e = et2.readByte();
        this.f = et2.readByte();
    }

    @Override
    public void b(et et2) {
        super.b(et2);
        et2.writeByte(this.e);
        et2.writeByte(this.f);
    }

    @Override
    public String b() {
        return super.b() + String.format(", yRot=%d, xRot=%d", this.e, this.f);
    }
}

