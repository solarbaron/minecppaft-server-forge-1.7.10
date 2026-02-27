/*
 * Decompiled with CFR 0.152.
 */
public class gm
extends ft {
    private int a;

    public gm() {
    }

    public gm(int n2) {
        this.a = n2;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
    }
}

