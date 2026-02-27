/*
 * Decompiled with CFR 0.152.
 */
public class iu
extends ft {
    private String a;
    private int b;
    private zb c;
    private boolean d;
    private rd e;
    private boolean f;

    @Override
    public void a(et et2) {
        this.a = et2.c(7);
        this.b = et2.readByte();
        this.c = zb.a(et2.readByte());
        this.d = et2.readBoolean();
        this.e = rd.a(et2.readByte());
        this.f = et2.readBoolean();
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeByte(this.b);
        et2.writeByte(this.c.a());
        et2.writeBoolean(this.d);
        et2.writeByte(this.e.a());
        et2.writeBoolean(this.f);
    }

    public void a(io io2) {
        io2.a(this);
    }

    public String c() {
        return this.a;
    }

    public int d() {
        return this.b;
    }

    public zb e() {
        return this.c;
    }

    public boolean f() {
        return this.d;
    }

    public rd g() {
        return this.e;
    }

    public boolean h() {
        return this.f;
    }

    @Override
    public String b() {
        return String.format("lang='%s', view=%d, chat=%s, col=%b, difficulty=%s, cape=%b", new Object[]{this.a, this.b, this.c, this.d, this.e, this.f});
    }
}

