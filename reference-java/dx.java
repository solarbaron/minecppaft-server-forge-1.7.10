/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public class dx
extends dy {
    private String b;

    public dx() {
        this.b = "";
    }

    public dx(String string) {
        this.b = string;
        if (string == null) {
            throw new IllegalArgumentException("Empty string not allowed");
        }
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeUTF(this.b);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        this.b = dataInput.readUTF();
        ds2.a(16 * this.b.length());
    }

    @Override
    public byte a() {
        return 8;
    }

    @Override
    public String toString() {
        return "\"" + this.b + "\"";
    }

    @Override
    public dy b() {
        return new dx(this.b);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dx dx2 = (dx)object;
            return this.b == null && dx2.b == null || this.b != null && this.b.equals(dx2.b);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ this.b.hashCode();
    }

    @Override
    public String a_() {
        return this.b;
    }
}

