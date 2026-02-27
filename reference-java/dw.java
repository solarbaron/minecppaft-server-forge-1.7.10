/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public class dw
extends dz {
    private short b;

    public dw() {
    }

    public dw(short s2) {
        this.b = s2;
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeShort(this.b);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        ds2.a(16L);
        this.b = dataInput.readShort();
    }

    @Override
    public byte a() {
        return 2;
    }

    @Override
    public String toString() {
        return "" + this.b + "s";
    }

    @Override
    public dy b() {
        return new dw(this.b);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dw dw2 = (dw)object;
            return this.b == dw2.b;
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ this.b;
    }

    @Override
    public long c() {
        return this.b;
    }

    @Override
    public int d() {
        return this.b;
    }

    @Override
    public short e() {
        return this.b;
    }

    @Override
    public byte f() {
        return (byte)(this.b & 0xFF);
    }

    @Override
    public double g() {
        return this.b;
    }

    @Override
    public float h() {
        return this.b;
    }
}

