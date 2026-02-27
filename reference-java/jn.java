/*
 * Decompiled with CFR 0.152.
 */
public class jn
extends ft {
    private int a;
    private int b;
    private int c;
    private String[] d;

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = et2.readShort();
        this.c = et2.readInt();
        this.d = new String[4];
        for (int i2 = 0; i2 < 4; ++i2) {
            this.d[i2] = et2.c(15);
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeShort(this.b);
        et2.writeInt(this.c);
        for (int i2 = 0; i2 < 4; ++i2) {
            et2.a(this.d[i2]);
        }
    }

    public void a(io io2) {
        io2.a(this);
    }

    public int c() {
        return this.a;
    }

    public int d() {
        return this.b;
    }

    public int e() {
        return this.c;
    }

    public String[] f() {
        return this.d;
    }
}

