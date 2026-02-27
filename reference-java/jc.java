/*
 * Decompiled with CFR 0.152.
 */
public class jc
extends ft {
    private int a;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
    }

    @Override
    public boolean a() {
        return true;
    }

    public int c() {
        return this.a;
    }
}

