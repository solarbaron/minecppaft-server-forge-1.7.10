/*
 * Decompiled with CFR 0.152.
 */
public class ix
extends ft {
    private int a;
    private int b;
    private int c;
    private short d;
    private add e;
    private int f;

    public void a(io io2) {
        io2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
        this.b = et2.readShort();
        this.c = et2.readByte();
        this.d = et2.readShort();
        this.f = et2.readByte();
        this.e = et2.c();
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.writeShort(this.b);
        et2.writeByte(this.c);
        et2.writeShort(this.d);
        et2.writeByte(this.f);
        et2.a(this.e);
    }

    @Override
    public String b() {
        if (this.e != null) {
            return String.format("id=%d, slot=%d, button=%d, type=%d, itemid=%d, itemcount=%d, itemaux=%d", this.a, this.b, this.c, this.f, adb.b(this.e.b()), this.e.b, this.e.k());
        }
        return String.format("id=%d, slot=%d, button=%d, type=%d, itemid=-1", this.a, this.b, this.c, this.f);
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

    public short f() {
        return this.d;
    }

    public add g() {
        return this.e;
    }

    public int h() {
        return this.f;
    }
}

