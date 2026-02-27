/*
 * Decompiled with CFR 0.152.
 */
public class jg
extends jd {
    public jg() {
        this.i = true;
    }

    @Override
    public void a(et et2) {
        this.e = et2.readFloat();
        this.f = et2.readFloat();
        super.a(et2);
    }

    @Override
    public void b(et et2) {
        et2.writeFloat(this.e);
        et2.writeFloat(this.f);
        super.b(et2);
    }
}

