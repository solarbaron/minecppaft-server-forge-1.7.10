/*
 * Decompiled with CFR 0.152.
 */
public class hr
extends ft {
    private int a;
    private int b;

    public hr() {
    }

    public hr(int n2, rw rw2) {
        this.a = n2;
        this.b = rw2.a();
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readUnsignedByte();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

