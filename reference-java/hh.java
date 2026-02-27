/*
 * Decompiled with CFR 0.152.
 */
public class hh
extends hf {
    public hh() {
        this.g = true;
    }

    public hh(int n2, byte by2, byte by3, byte by4, byte by5, byte by6) {
        super(n2);
        this.b = by2;
        this.c = by3;
        this.d = by4;
        this.e = by5;
        this.f = by6;
        this.g = true;
    }

    @Override
    public void a(et et2) {
        super.a(et2);
        this.b = et2.readByte();
        this.c = et2.readByte();
        this.d = et2.readByte();
        this.e = et2.readByte();
        this.f = et2.readByte();
    }

    @Override
    public void b(et et2) {
        super.b(et2);
        et2.writeByte(this.b);
        et2.writeByte(this.c);
        et2.writeByte(this.d);
        et2.writeByte(this.e);
        et2.writeByte(this.f);
    }

    @Override
    public String b() {
        return super.b() + String.format(", xa=%d, ya=%d, za=%d, yRot=%d, xRot=%d", this.b, this.c, this.d, this.e, this.f);
    }
}

