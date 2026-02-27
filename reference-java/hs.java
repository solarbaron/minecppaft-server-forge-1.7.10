/*
 * Decompiled with CFR 0.152.
 */
public class hs
extends ft {
    private int a;
    private rd b;
    private ahk c;
    private ahm d;

    public hs() {
    }

    public hs(int n2, rd rd2, ahm ahm2, ahk ahk2) {
        this.a = n2;
        this.b = rd2;
        this.c = ahk2;
        this.d = ahm2;
    }

    public void a(fv fv2) {
        fv2.a(this);
    }

    @Override
    public void a(et et2) {
        this.a = et2.readInt();
        this.b = rd.a(et2.readUnsignedByte());
        this.c = ahk.a(et2.readUnsignedByte());
        this.d = ahm.a(et2.c(16));
        if (this.d == null) {
            this.d = ahm.b;
        }
    }

    @Override
    public void b(et et2) {
        et2.writeInt(this.a);
        et2.writeByte(this.b.a());
        et2.writeByte(this.c.a());
        et2.a(this.d.a());
    }
}

