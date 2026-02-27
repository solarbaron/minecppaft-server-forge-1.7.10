/*
 * Decompiled with CFR 0.152.
 */
import java.security.PrivateKey;
import javax.crypto.SecretKey;

public class jx
extends ft {
    private byte[] a = new byte[0];
    private byte[] b = new byte[0];

    @Override
    public void a(et et2) {
        this.a = jx.a(et2);
        this.b = jx.a(et2);
    }

    @Override
    public void b(et et2) {
        jx.a(et2, this.a);
        jx.a(et2, this.b);
    }

    public void a(jv jv2) {
        jv2.a(this);
    }

    public SecretKey a(PrivateKey privateKey) {
        return pt.a(privateKey, this.a);
    }

    public byte[] b(PrivateKey privateKey) {
        if (privateKey == null) {
            return this.b;
        }
        return pt.b(privateKey, this.b);
    }
}

