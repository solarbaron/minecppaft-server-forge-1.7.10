/*
 * Decompiled with CFR 0.152.
 */
public class ig
extends ft {
    private int a;
    private int b;
    private int c;

    public ig() {
    }

    public ig(int n2, int n3, int n4) {
        this.a = n2;
        this.b = n3;
        this.c = n4;
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

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public boolean a() {
        return false;
    }

    @Override
    public String b() {
        return String.format("x=%d, y=%d, z=%d", this.a, this.b, this.c);
    }
}

