/*
 * Decompiled with CFR 0.152.
 */
public class jl
extends ft {
    private int a;

    @Override
    public void a(et et2) {
        this.a = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.writeShort(this.a);
    }

    public void a(io io2) {
        io2.a(this);
    }

    public int c() {
        return this.a;
    }
}

