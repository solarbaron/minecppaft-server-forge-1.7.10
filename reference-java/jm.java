/*
 * Decompiled with CFR 0.152.
 */
public class jm
extends ft {
    private int a;
    private add b;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readShort();
        this.b = et2.c();
    }

    @Override
    public void b(et et2) {
        et2.writeShort(this.a);
        et2.a(this.b);
    }

    public int c() {
        return this.a;
    }

    public add d() {
        return this.b;
    }
}

