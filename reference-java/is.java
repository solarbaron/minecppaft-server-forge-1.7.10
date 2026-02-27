/*
 * Decompiled with CFR 0.152.
 */
public class is
extends ft {
    private it a;

    public is() {
    }

    public is(it it2) {
        this.a = it2;
    }

    @Override
    public void a(et et2) {
        this.a = it.a()[et2.readByte() % it.a().length];
    }

    @Override
    public void b(et et2) {
        et2.writeByte(it.a(this.a));
    }

    public void a(io io2) {
        io2.a(this);
    }

    public it c() {
        return this.a;
    }
}

