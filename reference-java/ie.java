/*
 * Decompiled with CFR 0.152.
 */
public class ie
extends ft {
    private String a = "";
    private String b = "";
    private int c;
    private int d;

    public ie() {
    }

    public ie(azz azz2, int n2) {
        this.a = azz2.e();
        this.b = azz2.d().b();
        this.c = azz2.c();
        this.d = n2;
    }

    public ie(String string) {
        this.a = string;
        this.b = "";
        this.c = 0;
        this.d = 1;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(16);
        this.d = et2.readByte();
        if (this.d != 1) {
            this.b = et2.c(16);
            this.c = et2.readInt();
        }
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        et2.writeByte(this.d);
        if (this.d != 1) {
            et2.a(this.b);
            et2.writeInt(this.c);
        }
    }

    public void a(fv fv2) {
        fv2.a(this);
    }
}

