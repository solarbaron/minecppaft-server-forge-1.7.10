/*
 * Decompiled with CFR 0.152.
 */
public class ic
extends ft {
    private String a;
    private String b;
    private int c;

    public ic() {
    }

    public ic(azx azx2, int n2) {
        this.a = azx2.b();
        this.b = azx2.d();
        this.c = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(16);
        this.b = et2.c(32);
        this.c = et2.readByte();
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.a(this.b);
        et2.writeByte(this.c);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

