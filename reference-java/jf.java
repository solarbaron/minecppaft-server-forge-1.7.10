/*
 * Decompiled with CFR 0.152.
 */
public class jf
extends jd {
    public jf() {
        this.h = true;
        this.i = true;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readDouble();
        this.b = et2.readDouble();
        this.d = et2.readDouble();
        this.c = et2.readDouble();
        this.e = et2.readFloat();
        this.f = et2.readFloat();
        super.a(et2);
    }

    @Override
    public void b(et et2) {
        et2.writeDouble(this.a);
        et2.writeDouble(this.b);
        et2.writeDouble(this.d);
        et2.writeDouble(this.c);
        et2.writeFloat(this.e);
        et2.writeFloat(this.f);
        super.b(et2);
    }
}

