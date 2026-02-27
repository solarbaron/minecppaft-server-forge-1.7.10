/*
 * Decompiled with CFR 0.152.
 */
public class hv
extends ft {
    private int a;
    private String b;

    public hv() {
    }

    public hv(int n2, azx azx2) {
        this.a = n2;
        this.b = azx2 == null ? "" : azx2.b();
    }

    @Override
    public void a(et et2) {
        this.a = et2.readByte();
        this.b = et2.c(16);
    }

    @Override
    public void b(et et2) {
        et2.writeByte(this.a);
        et2.a(this.b);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

