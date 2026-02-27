/*
 * Decompiled with CFR 0.152.
 */
public class hd
extends ft {
    private int a;
    private boolean b;
    private ahk c;
    private int d;
    private rd e;
    private int f;
    private ahm g;

    public hd() {
    }

    public hd(int n2, ahk ahk2, boolean bl2, int n3, rd rd2, int n4, ahm ahm2) {
        this.a = n2;
        this.d = n3;
        this.e = rd2;
        this.c = ahk2;
        this.f = n4;
        this.b = bl2;
        this.g = ahm2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        int n2 = et2.readUnsignedByte();
        this.b = (n2 & 8) == 8;
        this.c = ahk.a(n2 &= 0xFFFFFFF7);
        this.d = et2.readByte();
        this.e = rd.a(et2.readUnsignedByte());
        this.f = et2.readUnsignedByte();
        this.g = ahm.a(et2.c(16));
        if (this.g == null) {
            this.g = ahm.b;
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        int n2 = this.c.a();
        if (this.b) {
            n2 |= 8;
        }
        et2.writeByte(n2);
        et2.writeByte(this.d);
        et2.writeByte(this.e.a());
        et2.writeByte(this.f);
        et2.a(this.g.a());
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public String b() {
        return String.format("eid=%d, gameType=%d, hardcore=%b, dimension=%d, difficulty=%s, maxplayers=%d", new Object[]{this.a, this.c.a(), this.b, this.d, this.e, this.f});
    }
}

