/*
 * Decompiled with CFR 0.152.
 */
public class ja
extends ft {
    private int a;
    private jb b;

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = jb.a()[et2.readByte() % jb.a().length];
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(jb.a(this.b));
    }

    public void a(io io2) {
        io2.a(this);
    }

    public sa a(ahb ahb2) {
        return ahb2.a(this.a);
    }

    public jb c() {
        return this.b;
    }
}

