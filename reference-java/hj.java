/*
 * Decompiled with CFR 0.152.
 */
public class hj
extends ft {
    private int a;
    private int b;
    private int c;

    public hj() {
    }

    public hj(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readInt();
        this.c = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
    }
}

