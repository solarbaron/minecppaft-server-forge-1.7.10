/*
 * Decompiled with CFR 0.152.
 */
public class ho
extends ft {
    private String a;
    private boolean b;
    private int c;

    public ho() {
    }

    public ho(String string, boolean bl2, int n2) {
        this.a = string;
        this.b = bl2;
        this.c = n2;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(16);
        this.b = et2.readBoolean();
        this.c = et2.readShort();
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeBoolean(this.b);
        et2.writeShort(this.c);
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

