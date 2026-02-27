/*
 * Decompiled with CFR 0.152.
 */
import java.io.DataInput;
import java.io.DataOutput;

public class dm
extends dz {
    private float b;

    dm() {
    }

    public dm(float f2) {
        this.b = f2;
    }

    @Override
    void a(DataOutput dataOutput) {
        dataOutput.writeFloat(this.b);
    }

    @Override
    void a(DataInput dataInput, int n2, ds ds2) {
        ds2.a(32L);
        this.b = dataInput.readFloat();
    }

    @Override
    public byte a() {
        return 5;
    }

    @Override
    public String toString() {
        return "" + this.b + "f";
    }

    @Override
    public dy b() {
        return new dm(this.b);
    }

    @Override
    public boolean equals(Object object) {
        if (super.equals(object)) {
            dm dm2 = (dm)object;
            return this.b == dm2.b;
        }
        return false;
    }

    @Override
    public int hashCode() {
        return super.hashCode() ^ Float.floatToIntBits(this.b);
    }

    @Override
    public long c() {
        return (long)this.b;
    }

    @Override
    public int d() {
        return qh.d(this.b);
    }

    @Override
    public short e() {
        return (short)(qh.d(this.b) & 0xFFFF);
    }

    @Override
    public byte f() {
        return (byte)(qh.d(this.b) & 0xFF);
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

