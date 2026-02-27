/*
 * Decompiled with CFR 0.152.
 */
public class ij
extends ft {
    private int a;
    private int b;

    public ij() {
    }

    public ij(int n2, int n3) {
        this.a = n2;
        this.b = n3;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

