/*
 * Decompiled with CFR 0.152.
 */
public class hf
extends ft {
    protected int a;
    protected byte b;
    protected byte c;
    protected byte d;
    protected byte e;
    protected byte f;
    protected boolean g;

    public hf() {
    }

    public hf(int n2) {
        this.a = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d", this.a);
    }

    @Override
    public String toString() {
        return "Entity_" + super.toString();
    }
}

