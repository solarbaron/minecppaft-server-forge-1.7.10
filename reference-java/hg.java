/*
 * Decompiled with CFR 0.152.
 */
public class hg
extends hf {
    public hg() {
    }

    public hg(int n2, byte by2, byte by3, byte by4) {
        super(n2);
        this.b = by2;
        this.c = by3;
        this.d = by4;
    }

    @Override
    public void a(et et2) {
        super.a(et2);
        this.b = et2.readByte();
        this.c = et2.readByte();
        this.d = et2.readByte();
    }

    @Override
    public void b(et et2) {
        super.b(et2);
        et2.writeByte(this.b);
        et2.writeByte(this.c);
        et2.writeByte(this.d);
    }

    @Override
    public String b() {
        return super.b() + String.format(", xa=%d, ya=%d, za=%d", this.b, this.c, this.d);
    }
}

