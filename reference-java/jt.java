/*
 * Decompiled with CFR 0.152.
 */
import java.security.PublicKey;

public class jt
extends ft {
    private String a;
    private PublicKey b;
    private byte[] c;

    public jt() {
    }

    public jt(String string, PublicKey publicKey, byte[] byArray) {
        this.a = string;
        this.b = publicKey;
        this.c = byArray;
    }

    @Override
    public void a(et et2) {
        this.a = et2.c(20);
        this.b = pt.a(jt.a(et2));
        this.c = jt.a(et2);
    }

    @Override
    public void b(et et2) {
        et2.a(this.a);
        jt.a(et2, this.b.getEncoded());
        jt.a(et2, this.c);
    }

    public void a(jr jr2) {
        jr2.a(this);
    }
}

