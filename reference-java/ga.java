/*
 * Decompiled with CFR 0.152.
 */
public class ga
extends ft {
    private int a;
    private int b;
    private int c;
    private int d;
    private int e;
    private String f;

    public ga() {
    }

    public ga(tb tb2) {
        this.a = tb2.y();
        this.b = tb2.b;
        this.c = tb2.c;
        this.d = tb2.d;
        this.e = tb2.a;
        this.f = tb2.e.B;
    }

    @Override
    public void a(et et2) {
        this.a = et2.a();
        this.f = et2.c(tc.A);
        this.b = et2.readInt();
        this.c = et2.readInt();
        this.d = et2.readInt();
        this.e = et2.readInt();
    }

    @Override
    public void b(et et2) {
        et2.b(this.a);
        et2.a(this.f);
        et2.writeInt(this.b);
        et2.writeInt(this.c);
        et2.writeInt(this.d);
        et2.writeInt(this.e);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("id=%d, type=%s, x=%d, y=%d, z=%d", this.a, this.f, this.b, this.c, this.d);
    }
}

