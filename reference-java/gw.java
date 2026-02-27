/*
 * Decompiled with CFR 0.152.
 */
public class gw
extends ft {
    private int a;

    public gw() {
    }

    public gw(int n2) {
        this.a = n2;
    }

    public void a(fv fv2) {
        fv2.a(this);
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
}

