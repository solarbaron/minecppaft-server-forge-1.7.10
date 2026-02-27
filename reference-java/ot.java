/*
 * Decompiled with CFR 0.152.
 */
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;

public class ot {
    private ByteArrayOutputStream a;
    private DataOutputStream b;

    public ot(int n2) {
        this.a = new ByteArrayOutputStream(n2);
        this.b = new DataOutputStream(this.a);
    }

    public void a(byte[] byArray) {
        this.b.write(byArray, 0, byArray.length);
    }

    public void a(String string) {
        this.b.writeBytes(string);
        this.b.write(0);
    }

    public void a(int n2) {
        this.b.write(n2);
    }

    public void a(short s2) {
        this.b.writeShort(Short.reverseBytes(s2));
    }

    public byte[] a() {
        return this.a.toByteArray();
    }

    public void b() {
        this.a.reset();
    }
}

