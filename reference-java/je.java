/*
 * Decompiled with CFR 0.152.
 */
public class je
extends jd {
    public je() {
        this.h = true;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readDouble();
        this.b = et2.readDouble();
        this.d = et2.readDouble();
        this.c = et2.readDouble();
        super.a(et2);
    }

    @Override
    public void b(et et2) {
        et2.writeDouble(this.a);
        et2.writeDouble(this.b);
        et2.writeDouble(this.d);
        et2.writeDouble(this.c);
        super.b(et2);
    }
}

