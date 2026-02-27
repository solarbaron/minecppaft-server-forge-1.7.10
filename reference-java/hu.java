/*
 * Decompiled with CFR 0.152.
 */
public class hu
extends ft {
    private int a;

    public hu() {
    }

    public hu(int n2) {
        this.a = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

